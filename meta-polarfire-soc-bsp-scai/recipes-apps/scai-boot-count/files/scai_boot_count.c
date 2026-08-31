#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/ioctl.h>

#ifdef DEBUG
#define CMDLINE_PATH            "cmdline"
#else
#define CMDLINE_PATH            "/proc/cmdline"
#endif

#define CMDLINE_SIZE            4096

#define MOUNT_CHECK_INTERVAL_MS 100
#define MOUNT_CHECK_TIMEOUT_MS  120*1000

#define BOOT_COUNT_FILE         "boot_count"
#define BOOT_COUNT_TMP_FILE     "boot_count.tmp"

#define STATUS_DIR              "/run/boot-count"

/*
 * Scratch buffer for the single telemetry request issued at the end of the
 * run. Only the side effect of the request matters, so it just has to be
 * large enough not to truncate a concise telemetry payload.
 */
#define TM_BUFFER_SIZE          4096

/*
 * --------------------------------------------------------------------------
 * Device ID
 * --------------------------------------------------------------------------
 *
 * Mapping:
 *
 *     ubi_a + nom -> NOM_MSS_W25
 *     ubi_b + nom -> NOM_FPGA_W25
 *     ubi_a + red -> RED_MSS_W25
 *     ubi_b + red -> RED_FPGA_W25
 */
enum device_id {
	NOM_MSS_W25 = 0,
	NOM_FPGA_W25,
	RED_MSS_W25,
	RED_FPGA_W25,
	BOOT_DEVICE_MAX
};

/*
 * --------------------------------------------------------------------------
 * Boot count
 * --------------------------------------------------------------------------
 *
 * Local storage:
 *
 *     boot_device[4] = 4 bytes
 *
 * Shared storage:
 *
 *     boot_device[4] = 4 bytes
 *     last_device    = 1 byte
 *
 * Total:
 *
 *     local  = 4 bytes
 *     shared = 5 bytes
 *
 * Each counter is uint8_t.
 */
struct boot_count {
	uint8_t boot_device[BOOT_DEVICE_MAX];
	uint8_t last_device;
};

/*
 * --------------------------------------------------------------------------
 * Context
 * --------------------------------------------------------------------------
 */
struct boot_count_context {
	char mount_point[PATH_MAX];

	char boot_count_path[PATH_MAX];
	char boot_count_tmp_path[PATH_MAX];

	char status_path[PATH_MAX];
	char status_tmp_path[PATH_MAX];

	bool is_shared_device;
};

/*
 * --------------------------------------------------------------------------
 * Device ID -> string
 * --------------------------------------------------------------------------
 */
static const char *device_id_name(enum device_id id)
{
	switch (id) {
	case NOM_MSS_W25:
		return "NOM_MSS_W25";

	case NOM_FPGA_W25:
		return "NOM_FPGA_W25";

	case RED_MSS_W25:
		return "RED_MSS_W25";

	case RED_FPGA_W25:
		return "RED_FPGA_W25";

	default:
		return "UNKNOWN";
	}
}

/*
 * --------------------------------------------------------------------------
 * Read exactly SIZE bytes
 * --------------------------------------------------------------------------
 */
static int read_full(int fd, void *buf, size_t size)
{
	size_t total = 0;

	while (total < size) {
		ssize_t ret;

		ret = read(fd,
				(uint8_t *)buf + total,
				size - total);

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			return -errno;
		}

		if (ret == 0)
			return -EIO;

		total += (size_t)ret;
	}

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Write exactly SIZE bytes
 * --------------------------------------------------------------------------
 */
static int write_full(int fd, const void *buf, size_t size)
{
	size_t total = 0;

	while (total < size) {
		ssize_t ret;

		ret = write(fd,
				(const uint8_t *)buf + total,
				size - total);

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			return -errno;
		}

		if (ret == 0)
			return -EIO;

		total += (size_t)ret;
	}

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Initialize context
 * --------------------------------------------------------------------------
 *
 * status_name is used to prevent status-file collision between services.
 *
 * Example:
 *
 *     status_name = "mss"
 *
 * produces:
 *
 *     /run/boot-count/mss.status
 *     /run/boot-count/mss.status.tmp
 */
static int init_context(struct boot_count_context *ctx,
		const char *mount_point,
		const char *status_name,
		bool is_shared_device)
{
	int ret;

	if (!ctx || !mount_point || !status_name)
		return -EINVAL;

	if (status_name[0] == '\0')
		return -EINVAL;

	memset(ctx, 0, sizeof(*ctx));

	/*
	 * Mount point
	 */
	ret = snprintf(ctx->mount_point,
			sizeof(ctx->mount_point),
			"%s",
			mount_point);

	if (ret < 0 || (size_t)ret >= sizeof(ctx->mount_point))
		return -ENAMETOOLONG;

	/*
	 * Persistent boot_count
	 */
	ret = snprintf(ctx->boot_count_path,
			sizeof(ctx->boot_count_path),
			"%s/%s",
			mount_point,
			BOOT_COUNT_FILE);

	if (ret < 0 || (size_t)ret >= sizeof(ctx->boot_count_path))
		return -ENAMETOOLONG;

	/*
	 * Temporary boot_count
	 */
	ret = snprintf(ctx->boot_count_tmp_path,
			sizeof(ctx->boot_count_tmp_path),
			"%s/%s",
			mount_point,
			BOOT_COUNT_TMP_FILE);

	if (ret < 0 || (size_t)ret >= sizeof(ctx->boot_count_tmp_path))
		return -ENAMETOOLONG;

	/*
	 * Runtime status file
	 */
	ret = snprintf(ctx->status_path,
			sizeof(ctx->status_path),
			"%s/%s.status",
			STATUS_DIR,
			status_name);

	if (ret < 0 || (size_t)ret >= sizeof(ctx->status_path))
		return -ENAMETOOLONG;

	/*
	 * Runtime status temporary file
	 */
	ret = snprintf(ctx->status_tmp_path,
			sizeof(ctx->status_tmp_path),
			"%s/%s.status.tmp",
			STATUS_DIR,
			status_name);

	if (ret < 0 || (size_t)ret >= sizeof(ctx->status_tmp_path))
		return -ENAMETOOLONG;

	ctx->is_shared_device = is_shared_device;

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Check whether mount point is REALLY mounted
 * --------------------------------------------------------------------------
 *
 * /proc/self/mountinfo is used.
 *
 * We intentionally do not use access() or stat() alone because the
 * directory can exist even when the filesystem itself is not mounted.
 */
static int is_mount_ready(const char *mount_point)
{
	FILE *fp;
	char line[4096];

	if (!mount_point)
		return 0;

	fp = fopen("/proc/self/mountinfo", "r");

	if (!fp)
		return 0;

	while (fgets(line, sizeof(line), fp)) {
		char *saveptr = NULL;
		char *token;
		char mount_path[PATH_MAX];
		int field = 0;

		token = strtok_r(line, " ", &saveptr);

		while (token) {
			field++;

			/*
			 * mountinfo format:
			 *
			 * 1  mount ID
			 * 2  parent ID
			 * 3  major:minor
			 * 4  root
			 * 5  mount point
			 */
			if (field == 5) {
				snprintf(mount_path,
						sizeof(mount_path),
						"%s",
						token);
				break;
			}

			token = strtok_r(NULL, " ", &saveptr);
		}

		if (field == 5 &&
				strcmp(mount_path, mount_point) == 0) {
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Wait until mount point is ready
 * --------------------------------------------------------------------------
 */
static int wait_for_mount(const char *mount_point)
{
	int elapsed = 0;

	while (elapsed < MOUNT_CHECK_TIMEOUT_MS) {
		if (is_mount_ready(mount_point)) {
			printf("Mount is ready: %s\n",
					mount_point);

			return 0;
		}

		usleep(MOUNT_CHECK_INTERVAL_MS * 1000);
		elapsed += MOUNT_CHECK_INTERVAL_MS;
	}

	fprintf(stderr,
			"Timeout waiting for mount: %s\n",
			mount_point);

	return -ETIMEDOUT;
}

/*
 * --------------------------------------------------------------------------
 * Ensure runtime status directory
 * --------------------------------------------------------------------------
 */
static int ensure_status_dir(void)
{
	if (mkdir(STATUS_DIR, 0755) != 0) {
		if (errno != EEXIST)
			return -errno;
	}

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Write runtime status atomically
 * --------------------------------------------------------------------------
 *
 * Status:
 *
 *     WAITING_MOUNT
 *     UPDATING
 *     READY
 *     FAILED
 *
 * The status is stored under /run, not under the persistent mount.
 *
 * This is important because WAITING_MOUNT must be representable even
 * before the persistent filesystem is mounted.
 */
static int write_status(const struct boot_count_context *ctx,
		const char *status)
{
	int fd;
	int ret;
	size_t len;
	char buffer[32];

	if (!ctx || !status)
		return -EINVAL;

	ret = ensure_status_dir();

	if (ret != 0)
		return ret;

	ret = snprintf(buffer,
			sizeof(buffer),
			"%s\n",
			status);

	if (ret < 0 || (size_t)ret >= sizeof(buffer))
		return -EINVAL;

	len = (size_t)ret;

	/*
	 * Write temporary status file first.
	 */
	fd = open(ctx->status_tmp_path,
			O_WRONLY | O_CREAT | O_TRUNC,
			0644);

	if (fd < 0) {
		fprintf(stderr,
				"open(%s): %s\n",
				ctx->status_tmp_path,
				strerror(errno));

		return -errno;
	}

	ret = write_full(fd, buffer, len);

	if (ret != 0) {
		close(fd);
		unlink(ctx->status_tmp_path);

		return ret;
	}

	/*
	 * Make status contents persistent.
	 */
	if (fsync(fd) != 0) {
		ret = -errno;

		close(fd);
		unlink(ctx->status_tmp_path);

		return ret;
	}

	if (close(fd) != 0) {
		ret = -errno;

		unlink(ctx->status_tmp_path);

		return ret;
	}

	/*
	 * Atomic status update.
	 */
	if (rename(ctx->status_tmp_path,
			ctx->status_path) != 0) {
		ret = -errno;

		unlink(ctx->status_tmp_path);

		return ret;
	}

	printf("Status [%s]: %s\n",
			ctx->status_path,
			status);

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Parse kernel command line
 * --------------------------------------------------------------------------
 *
 * Expected command line:
 *
 *     ... ubi.mtd=ubi_b ... boot_device=red
 *
 * Mapping:
 *
 *     ubi_b + nom -> NOM_FPGA_W25
 *     ubi_a + nom -> NOM_MSS_W25
 *     ubi_b + red -> RED_FPGA_W25
 *     ubi_a + red -> RED_MSS_W25
 */
static int parse_cmdline(enum device_id *device)
{
	FILE *fp;
	char cmdline[CMDLINE_SIZE];

	char *token;
	char *saveptr = NULL;

	char ubi_mtd[32] = { 0 };
	char boot_device[32] = { 0 };

	bool found_ubi = false;
	bool found_boot_device = false;

	if (!device)
		return -EINVAL;

	*device = BOOT_DEVICE_MAX;

	fp = fopen(CMDLINE_PATH, "r");

	if (!fp) {
		fprintf(stderr,
				"fopen(%s): %s\n",
				CMDLINE_PATH,
				strerror(errno));

		return -errno;
	}

	if (!fgets(cmdline,
			sizeof(cmdline),
			fp)) {
		fclose(fp);
		return -EIO;
	}

	fclose(fp);

	token = strtok_r(cmdline,
			" \t\r\n",
			&saveptr);

	while (token) {
		if (strncmp(token, "ubi.mtd=", 8) == 0) {
			ret:
			snprintf(ubi_mtd,
					sizeof(ubi_mtd),
					"%s",
					token + 8);

			found_ubi = true;
		} else if (strncmp(token, "boot_device=", 12) == 0) {
			snprintf(boot_device,
					sizeof(boot_device),
					"%s",
					token + 12);

			found_boot_device = true;
		}

		token = strtok_r(NULL,
				" \t\r\n",
				&saveptr);
	}

	printf("ubi.mtd     : %s\n", ubi_mtd);
	printf("boot_device : %s\n", boot_device);

	if (!found_ubi || !found_boot_device) {
		fprintf(stderr,
				"Required boot parameters are missing\n");

		return -EINVAL;
	}

	/*
	 * ubi_b
	 */
	if (strcmp(ubi_mtd, "ubi_b") == 0) {
		if (strcmp(boot_device, "nom") == 0) {
			*device = NOM_FPGA_W25;
		} else if (strcmp(boot_device, "red") == 0) {
			*device = RED_FPGA_W25;
		} else {
			fprintf(stderr,
					"Invalid boot_device: %s\n",
					boot_device);

			return -EINVAL;
		}

	/*
	 * ubi_a
	 */
	} else if (strcmp(ubi_mtd, "ubi_a") == 0) {
		if (strcmp(boot_device, "nom") == 0) {
			*device = NOM_MSS_W25;
		} else if (strcmp(boot_device, "red") == 0) {
			*device = RED_MSS_W25;
		} else {
			fprintf(stderr,
					"Invalid boot_device: %s\n",
					boot_device);

			return -EINVAL;
		}

	} else {
		fprintf(stderr,
				"Invalid ubi.mtd: %s\n",
				ubi_mtd);

		return -EINVAL;
	}

	printf("Device ID   : %s (%d)\n",
			device_id_name(*device),
			*device);

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Load boot count
 * --------------------------------------------------------------------------
 *
 * Local:
 *
 *     4 bytes
 *
 * Shared:
 *
 *     5 bytes
 */
static int load_boot_count(const struct boot_count_context *ctx,
		struct boot_count *count)
{
	int fd;
	int ret;
	size_t expected_size;

	if (!ctx || !count)
		return -EINVAL;

	memset(count, 0, sizeof(*count));

	expected_size = sizeof(count->boot_device);

	if (ctx->is_shared_device)
		expected_size += sizeof(count->last_device);

	fd = open(ctx->boot_count_path, O_RDONLY);

	if (fd < 0) {
		/*
		 * First boot.
		 */
		if (errno == ENOENT) {
			printf("boot_count does not exist. "
					"Starting from zero.\n");

			return 0;
		}

		fprintf(stderr,
				"open(%s): %s\n",
				ctx->boot_count_path,
				strerror(errno));

		return -errno;
	}

	ret = read_full(fd, count, expected_size);

	if (ret != 0) {
		close(fd);

		fprintf(stderr,
				"Failed to read boot_count: %d\n",
				ret);

		return ret;
	}

	if (close(fd) != 0)
		return -errno;

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Save boot count atomically
 * --------------------------------------------------------------------------
 */
static int save_boot_count(const struct boot_count_context *ctx,
		const struct boot_count *count)
{
	int fd;
	int ret;
	int dir_fd;
	size_t count_size;

	if (!ctx || !count)
		return -EINVAL;

	count_size = sizeof(count->boot_device);

	if (ctx->is_shared_device)
		count_size += sizeof(count->last_device);

	/*
	 * Write temporary file.
	 */
	fd = open(ctx->boot_count_tmp_path,
			O_WRONLY | O_CREAT | O_TRUNC,
			0644);

	if (fd < 0) {
		fprintf(stderr,
				"open(%s): %s\n",
				ctx->boot_count_tmp_path,
				strerror(errno));

		return -errno;
	}

	ret = write_full(fd, count, count_size);

	if (ret != 0) {
		close(fd);
		unlink(ctx->boot_count_tmp_path);

		return ret;
	}

	/*
	 * Ensure boot_count contents reach storage.
	 */
	if (fsync(fd) != 0) {
		ret = -errno;

		close(fd);
		unlink(ctx->boot_count_tmp_path);

		return ret;
	}

	if (close(fd) != 0) {
		ret = -errno;

		unlink(ctx->boot_count_tmp_path);

		return ret;
	}

	/*
	 * Atomic replacement.
	 */
	if (rename(ctx->boot_count_tmp_path,
			ctx->boot_count_path) != 0) {
		ret = -errno;

		unlink(ctx->boot_count_tmp_path);

		return ret;
	}

	/*
	 * Ensure directory metadata containing the rename is persistent.
	 */
	dir_fd = open(ctx->mount_point,
			O_RDONLY | O_DIRECTORY);

	if (dir_fd >= 0) {
		if (fsync(dir_fd) != 0) {
			ret = -errno;
			close(dir_fd);
			return ret;
		}

		close(dir_fd);
	}

	return 0;
}

/*
 * --------------------------------------------------------------------------
 * Update boot count
 * --------------------------------------------------------------------------
 */
static int update_boot_count(const struct boot_count_context *ctx,
		enum device_id device)
{
	struct boot_count count;
	int ret;

	if (!ctx)
		return -EINVAL;

	if (device >= BOOT_DEVICE_MAX)
		return -EINVAL;

	ret = load_boot_count(ctx, &count);

	if (ret != 0)
		return ret;

	/*
	 * Do not allow uint8_t wrap-around:
	 *
	 *     255 -> 0
	 */
	if (count.boot_device[device] == UINT8_MAX) {
		fprintf(stderr,
				"Boot count overflow: %s\n",
				device_id_name(device));

		return -ERANGE;
	}

	count.boot_device[device]++;

	/*
	 * last_device exists only in shared storage.
	 */
	if (ctx->is_shared_device)
		count.last_device = (uint8_t)device;

	printf("Updating boot count:\n");
	printf("  device      = %s\n",
			device_id_name(device));
	printf("  count       = %u\n",
			count.boot_device[device]);

	if (ctx->is_shared_device) {
		printf("  last_device = %u (%s)\n",
				count.last_device,
				device_id_name(
					(enum device_id)count.last_device));
	}

	ret = save_boot_count(ctx, &count);

	if (ret != 0) {
		fprintf(stderr,
				"Failed to save boot_count: %d\n",
				ret);

		return ret;
	}

	return 0;
}

enum {
	SBI_EXT_TELEMETRY_RPROC_COMMAND = 0x14,
};

enum sbi_tm_ext_cmd {
	SBI_TM_EXT_CONCISE = 0x0,
	SBI_TM_EXT_VERBOSE = 0x1,
	SBI_TM_EXT_STOP_SERVICE = 0x2,
};

enum sbi_tm_ext_services {
	SBI_TM_EXT_STOP_NO_SERVICE = 0x0,
	SBI_TM_EXT_STOP_PUBLISHING = 0x1,
	SBI_TM_EXT_STOP_EXTERNAL_WDOG = 0x2,
};

struct user_data {
	unsigned long arg0;
	long arg1;
	void *buf;
};

static int stop_telemetry_publish(void)
{
	const char *dev = "/dev/scai_tm_rproc";
	struct user_data user_data = { 0 };
	unsigned int cmd = SBI_EXT_TELEMETRY_RPROC_COMMAND;
	int fd = -1;
	int ret = 0;

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("/dev/scai_tm_rproc open failed ...");
		ret = -1;
		return ret;
	}

	user_data.arg0 = SBI_TM_EXT_STOP_SERVICE;
	user_data.arg1 = SBI_TM_EXT_STOP_PUBLISHING;

	printf("Call ioctl:\n");
	printf("    user_data.arg0 : %d\n", user_data.arg0);
	printf("    user_data.arg1 : %d\n", user_data.arg1);

	if (ioctl(fd, cmd, &user_data) < 0) {
		perror("ioctl failed ...");
		ret = -1;
		goto out;
	}

out:
	if (fd >= 0)
		close(fd);

	return ret;
}

/*
 * --------------------------------------------------------------------------
 * Hand the external watchdog over to FSW
 * --------------------------------------------------------------------------
 *
 * A telemetry data request (SBI_TM_EXT_CONCISE / SBI_TM_EXT_VERBOSE) makes
 * HSS stop pinging the external watchdog on its own and ping it once on
 * behalf of the caller. From that point on the watchdog is serviced only by
 * telemetry requests coming from Linux.
 *
 * This request is issued once, at the very end of the boot count service, so
 * that FSW - which comes up right after it - takes the servicing over with
 * its own periodic telemetry requests before the external watchdog expires.
 *
 * The telemetry payload is not used here, only the side effect. The driver
 * copies min(HSS payload, user_data.arg1) bytes, so the local buffer cannot
 * be overrun.
 */
static int handover_external_wdog(void)
{
	const char *dev = "/dev/scai_tm_rproc";
	struct user_data user_data = { 0 };
	unsigned int cmd = SBI_EXT_TELEMETRY_RPROC_COMMAND;
	uint8_t buffer[TM_BUFFER_SIZE];
	int fd = -1;
	int ret = 0;

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("/dev/scai_tm_rproc open failed ...");
		ret = -1;
		return ret;
	}

	/*
	 * For the data commands arg1 is the caller buffer size, and the
	 * driver writes the number of copied bytes back into it.
	 */
	user_data.arg0 = SBI_TM_EXT_CONCISE;
	user_data.arg1 = (long)sizeof(buffer);
	user_data.buf = buffer;

	printf("Call ioctl (external watchdog handover):\n");
	printf("    user_data.arg0 : %lu\n", user_data.arg0);
	printf("    user_data.arg1 : %ld\n", user_data.arg1);

	if (ioctl(fd, cmd, &user_data) < 0) {
		perror("ioctl failed ...");
		ret = -1;
		goto out;
	}

	printf("External watchdog handed over to FSW "
			"(%ld telemetry bytes returned)\n",
			user_data.arg1);

out:
	if (fd >= 0)
		close(fd);

	return ret;
}

/*
 * --------------------------------------------------------------------------
 * Main
 * --------------------------------------------------------------------------
 *
 * Usage:
 *
 *     scai_boot_count <mount-point> <local|shared> <status-name>
 *
 * Examples:
 *
 *     scai_boot_count /opt/mount_device_extra local w25
 *
 *     scai_boot_count /opt/mount_device_extra_fpga shared mtd_a
 *
 *     scai_boot_count /opt/mount_3dplus shared mtd_b
 *
 * Runtime status files:
 *
 *     /run/boot-count/mss.status
 *     /run/boot-count/fpga.status
 *     /run/boot-count/3dplus.status
 */
int main(int argc, char *argv[])
{
	struct boot_count_context ctx;
	enum device_id device;
	bool is_shared;
	int ret;

	/*
	 * ----------------------------------------------------------------------
	 * Argument validation
	 * ----------------------------------------------------------------------
	 */
	if (argc != 4) {
		fprintf(stderr,
				"Usage: %s "
				"<mount-point> "
				"<local|shared> "
				"<status-name>\n",
				argv[0]);

		return EXIT_FAILURE;
	}

	/*
	 * ----------------------------------------------------------------------
	 * Storage type
	 * ----------------------------------------------------------------------
	 */
	if (strcmp(argv[2], "shared") == 0) {
		is_shared = true;
	} else if (strcmp(argv[2], "local") == 0) {
		is_shared = false;
	} else {
		fprintf(stderr,
				"Invalid storage type: %s\n",
				argv[2]);

		return EXIT_FAILURE;
	}

	/*
	 * ----------------------------------------------------------------------
	 * Initialize context
	 *
	 * No persistent filesystem access happens here.
	 * ----------------------------------------------------------------------
	 */
	ret = init_context(&ctx,
			argv[1],
			argv[3],
			is_shared);

	if (ret != 0) {
		fprintf(stderr,
				"Failed to initialize context: %d\n",
				ret);

		return EXIT_FAILURE;
	}

	printf("Mount point  : %s\n",
			ctx.mount_point);

	printf("Storage type : %s\n",
			ctx.is_shared_device ? "shared" : "local");

	printf("Status file  : %s\n",
			ctx.status_path);

	/*
	 * ----------------------------------------------------------------------
	 * Status: waiting for mount
	 *
	 * This does NOT touch the persistent filesystem.
	 * ----------------------------------------------------------------------
	 */
	ret = write_status(&ctx, "WAITING_MOUNT");

	if (ret != 0) {
		fprintf(stderr,
				"Failed to write WAITING_MOUNT status: %d\n",
				ret);
	}

	/*
	 * ----------------------------------------------------------------------
	 * Wait until persistent filesystem is actually mounted.
	 * ----------------------------------------------------------------------
	 */
	ret = wait_for_mount(ctx.mount_point);

	if (ret != 0) {
		fprintf(stderr,
				"Mount is not ready: %s\n",
				ctx.mount_point);

		write_status(&ctx, "FAILED");

		return EXIT_FAILURE;
	}

	/*
	 * ----------------------------------------------------------------------
	 * Persistent filesystem is now available.
	 * ----------------------------------------------------------------------
	 */
	ret = write_status(&ctx, "UPDATING");

	if (ret != 0) {
		fprintf(stderr,
				"Failed to write UPDATING status: %d\n",
				ret);

		return EXIT_FAILURE;
	}

	/*
	 * ----------------------------------------------------------------------
	 * Parse kernel command line.
	 * ----------------------------------------------------------------------
	 */
	ret = parse_cmdline(&device);

	if (ret != 0)
		goto failed;

	/*
	 * ----------------------------------------------------------------------
	 * Update persistent boot counter.
	 * ----------------------------------------------------------------------
	 */
	ret = update_boot_count(&ctx, device);

	if (ret != 0)
		goto failed;

	if (!ctx.is_shared_device) {
		stop_telemetry_publish();
	}
	/*
	 * ----------------------------------------------------------------------
	 * Update completed.
	 *
	 * Telemetry application can now safely read boot_count.
	 * ----------------------------------------------------------------------
	 */
	ret = write_status(&ctx, "READY");

	if (ret != 0) {
		fprintf(stderr,
				"Failed to write READY status: %d\n",
				ret);

		return EXIT_FAILURE;
	}

	printf("Boot count update completed successfully.\n");

	/*
	 * Last action of the service: HSS stops servicing the external
	 * watchdog here and FSW has to take over with its own telemetry
	 * requests before the watchdog expires.
	 */
	if (!ctx.is_shared_device) {
		if (handover_external_wdog() != 0) {
			fprintf(stderr,
					"Failed to hand the external watchdog "
					"over to FSW\n");

			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;

failed:

	/*
	 * Status is independent from persistent storage.
	 */
	if (write_status(&ctx, "FAILED") != 0) {
		fprintf(stderr,
				"Failed to write FAILED status\n");
	}

	return EXIT_FAILURE;
}
