#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define TM_BUFFER_SIZE     (16 * 1024 * 1024)

enum device_id {
	NOM_MSS_W25 = 0,
	NOM_FPGA_W25,
	RED_MSS_W25,
	RED_FPGA_W25,
	BOOT_DEVICE_MAX
};

#define BOOT_COUNT_STATUS_DIR              "/run/boot-count"
struct boot_count_w25 {
	uint8_t boot_device[BOOT_DEVICE_MAX];
};

struct boot_count_ext {
	uint8_t boot_device[BOOT_DEVICE_MAX];
	uint8_t last_device;
};

struct telemetry_data {
	int32_t temp_K;
	int32_t temp_C;
	int32_t input_mV;
	int32_t input_mA;
	int32_t rail_1_0_mV;
	int32_t rail_1_0_mA;
	int32_t rail_1_0_from_pf_mV;
	int32_t rail_1_2_mV;
	int32_t rail_1_2_mA;
	int32_t rail_1_8_mV;
	int32_t rail_1_8_mA;
	int32_t rail_1_8_from_pf_mV;
	int32_t rail_2_5_mV;
	int32_t rail_2_5_mA;
	int32_t rail_2_5_from_pf_mV;
	int32_t rail_3_3_mV;
	int32_t rail_3_3_mA;
	int32_t sddr_mV;
	int32_t fddr_mV;
	int32_t adc1_mV;
	int32_t adc2_mV;
	int32_t camera1_thermistor_mOhm_plus;
	int32_t camera1_thermistor_mOhm_minus;
	int32_t camera2_thermistor_mOhm_plus;
	int32_t camera2_thermistor_mOhm_minus;
	int32_t camera1_mV;
	int32_t camera2_mV;
	int32_t cams_telem_mV;
	int32_t adc_tel_mV;
	int32_t sanity_check_1_0_mV;
	int32_t sanity_check_1_0_mA;
	int32_t sanity_check_1_2_mV;
	int32_t sanity_check_1_2_mA;
	int32_t sanity_check_1_8_mV;
	int32_t sanity_check_1_8_mA;
	int32_t sanity_check_2_5_mV;
	int32_t sanity_check_2_5_mA;
	int32_t sanity_check_3_3_mV;
	int32_t sanity_check_3_3_mA;
	int32_t sanity_check_sddr_vtt;
	int32_t sanity_check_fddr_vtt;
	int32_t edac_cnt_ddrc;
	struct boot_count_w25 boot_count_w25;     // w25 -- main booting rootfs
	struct boot_count_ext boot_count_mtd[2];  // [0] == scai_mtd_a, in case of dpu460 == 3dplus_mtd_a
	                                          // [1] == scai_mtd_b, in case of dpu460 == 3dplus_mtd_b
};

struct user_data {
	unsigned long arg0;
	long size;
	void *buf;
};

enum {
	SBI_EXT_TELEMETRY_RPROC_COMMAND = 0x14,
};

enum sbi_tm_ext_cmd {
	SBI_TM_EXT_CONCISE = 0x0,
	SBI_TM_EXT_VERBOSE = 0x1,
	SBI_TM_EXT_STOP_SERVICE = 0x2,
};

static void print_tm_data(struct telemetry_data *tm_data)
{
	printf("tm_data->temp_K                        : %d\r\n",
			tm_data->temp_K);
	printf("tm_data->temp_C                        : %d\r\n",
			tm_data->temp_C);
	printf("tm_data->input_mV                      : %d\r\n",
			tm_data->input_mV);
	printf("tm_data->input_mA                      : %d\r\n",
			tm_data->input_mA);
	printf("tm_data->rail_1_0_mV                   : %d\r\n",
			tm_data->rail_1_0_mV);
	printf("tm_data->rail_1_0_mA                   : %d\r\n",
			tm_data->rail_1_0_mA);
	printf("tm_data->rail_1_0_from_pf_mV           : %d\r\n",
			tm_data->rail_1_0_from_pf_mV);
	printf("tm_data->rail_1_2_mV                   : %d\r\n",
			tm_data->rail_1_2_mV);
	printf("tm_data->rail_1_2_mA                   : %d\r\n",
			tm_data->rail_1_2_mA);
	printf("tm_data->rail_1_8_mV                   : %d\r\n",
			tm_data->rail_1_8_mV);
	printf("tm_data->rail_1_8_mA                   : %d\r\n",
			tm_data->rail_1_8_mA);
	printf("tm_data->rail_1_8_from_pf_mV           : %d\r\n",
			tm_data->rail_1_8_from_pf_mV);
	printf("tm_data->rail_2_5_mV                   : %d\r\n",
			tm_data->rail_2_5_mV);
	printf("tm_data->rail_2_5_mA                   : %d\r\n",
			tm_data->rail_2_5_mA);
	printf("tm_data->rail_2_5_from_pf_mV           : %d\r\n",
			tm_data->rail_2_5_from_pf_mV);
	printf("tm_data->rail_3_3_mV                   : %d\r\n",
			tm_data->rail_3_3_mV);
	printf("tm_data->rail_3_3_mA                   : %d\r\n",
			tm_data->rail_3_3_mA);
	printf("tm_data->sddr_mV                       : %d\r\n",
			tm_data->sddr_mV);
	printf("tm_data->fddr_mV                       : %d\r\n",
			tm_data->fddr_mV);
	printf("tm_data->adc1_mV                       : %d\r\n",
			tm_data->adc1_mV);
	printf("tm_data->adc2_mV                       : %d\r\n",
			tm_data->adc2_mV);
	printf("tm_data->camera1_thermistor_mOhm_plus  : %d\r\n",
			tm_data->camera1_thermistor_mOhm_plus);
	printf("tm_data->camera1_thermistor_mOhm_minus : %d\r\n",
			tm_data->camera1_thermistor_mOhm_minus);
	printf("tm_data->camera2_thermistor_mOhm_plus  : %d\r\n",
			tm_data->camera2_thermistor_mOhm_plus);
	printf("tm_data->camera2_thermistor_mOhm_minus : %d\r\n",
			tm_data->camera2_thermistor_mOhm_minus);
	printf("tm_data->camera1_mV                    : %d\r\n",
			tm_data->camera1_mV);
	printf("tm_data->camera2_mV                    : %d\r\n",
			tm_data->camera2_mV);
	printf("tm_data->cams_telem_mV                 : %d\r\n",
			tm_data->cams_telem_mV);
	printf("tm_data->adc_tel_mV                    : %d\r\n",
			tm_data->adc_tel_mV);
	printf("tm_data->sanity_check_1_0_mV           : %d\r\n",
			tm_data->sanity_check_1_0_mV);
	printf("tm_data->sanity_check_1_0_mA           : %d\r\n",
			tm_data->sanity_check_1_0_mA);
	printf("tm_data->sanity_check_1_2_mV           : %d\r\n",
			tm_data->sanity_check_1_2_mV);
	printf("tm_data->sanity_check_1_2_mA           : %d\r\n",
			tm_data->sanity_check_1_2_mA);
	printf("tm_data->sanity_check_1_8_mV           : %d\r\n",
			tm_data->sanity_check_1_8_mV);
	printf("tm_data->sanity_check_1_8_mA           : %d\r\n",
			tm_data->sanity_check_1_8_mA);
	printf("tm_data->sanity_check_2_5_mV           : %d\r\n",
			tm_data->sanity_check_2_5_mV);
	printf("tm_data->sanity_check_2_5_mA           : %d\r\n",
			tm_data->sanity_check_2_5_mA);
	printf("tm_data->sanity_check_3_3_mV           : %d\r\n",
			tm_data->sanity_check_3_3_mV);
	printf("tm_data->sanity_check_3_3_mA           : %d\r\n",
			tm_data->sanity_check_3_3_mA);
	printf("tm_data->sanity_check_sddr_vtt         : %d\r\n",
			tm_data->sanity_check_sddr_vtt);
	printf("tm_data->sanity_check_fddr_vtt         : %d\r\n",
			tm_data->sanity_check_fddr_vtt);

	printf("tm_data->edac_cnt_ddrc                 : %d\r\n",
			tm_data->edac_cnt_ddrc);

	printf("tm_data->boot_count_w25.boot_device[NOM_MSS_W25]     : 0x%02X\r\n",
			tm_data->boot_count_w25.boot_device[NOM_MSS_W25]);
	printf("tm_data->boot_count_w25.boot_device[NOM_FPGA_W25]    : 0x%02X\r\n",
			tm_data->boot_count_w25.boot_device[NOM_FPGA_W25]);
	printf("tm_data->boot_count_w25.boot_device[RED_MSS_W25]     : 0x%02X\r\n",
			tm_data->boot_count_w25.boot_device[RED_MSS_W25]);
	printf("tm_data->boot_count_w25.boot_device[RED_FPGA_W25]    : 0x%02X\r\n",
			tm_data->boot_count_w25.boot_device[RED_FPGA_W25]);

	printf("tm_data->boot_count_mtd[0].boot_device[NOM_MSS_W25]  : 0x%02X\r\n",
			tm_data->boot_count_mtd[0].boot_device[NOM_MSS_W25]);
	printf("tm_data->boot_count_mtd[0].boot_device[NOM_FPGA_W25] : 0x%02X\r\n",
			tm_data->boot_count_mtd[0].boot_device[NOM_FPGA_W25]);
	printf("tm_data->boot_count_mtd[0].boot_device[RED_MSS_W25]  : 0x%02X\r\n",
			tm_data->boot_count_mtd[0].boot_device[RED_MSS_W25]);
	printf("tm_data->boot_count_mtd[0].boot_device[RED_FPGA_W25] : 0x%02X\r\n",
			tm_data->boot_count_mtd[0].boot_device[RED_FPGA_W25]);
	printf("tm_data->boot_count_mtd[0].last_device               : 0x%02X\r\n",
			tm_data->boot_count_mtd[0].last_device);

	printf("tm_data->boot_count_mtd[1].boot_device[NOM_MSS_W25]  : 0x%02X\r\n",
			tm_data->boot_count_mtd[1].boot_device[NOM_MSS_W25]);
	printf("tm_data->boot_count_mtd[1].boot_device[NOM_FPGA_W25] : 0x%02X\r\n",
			tm_data->boot_count_mtd[1].boot_device[NOM_FPGA_W25]);
	printf("tm_data->boot_count_mtd[1].boot_device[RED_MSS_W25]  : 0x%02X\r\n",
			tm_data->boot_count_mtd[1].boot_device[RED_MSS_W25]);
	printf("tm_data->boot_count_mtd[1].boot_device[RED_FPGA_W25] : 0x%02X\r\n",
			tm_data->boot_count_mtd[1].boot_device[RED_FPGA_W25]);
	printf("tm_data->boot_count_mtd[1].last_device               : 0x%02X\r\n",
			tm_data->boot_count_mtd[1].last_device);
}

struct boot_count_entry {
	const char *status_file;
	const char *boot_count_file;
	void *boot_count;
	size_t boot_count_size;
};

int set_boot_count(struct telemetry_data *p_tm_data)
{
	char buf[256];

	struct boot_count_entry entries[] = {
		{
			"/run/boot-count/w25.status",
			"/boot_count",
			&p_tm_data->boot_count_w25,
			sizeof(p_tm_data->boot_count_w25)
		},
		{
			"/run/boot-count/mtd_a.status",
			"/opt/scai_mtd_a/boot_count",
			&p_tm_data->boot_count_mtd[0],
			sizeof(p_tm_data->boot_count_mtd[0])
		},
		{
			"/run/boot-count/mtd_b.status",
			"/opt/scai_mtd_b/boot_count",
			&p_tm_data->boot_count_mtd[1],
			sizeof(p_tm_data->boot_count_mtd[1])
		},
	};

	for (size_t i = 0; i < sizeof(entries) / sizeof(entries[0]); i++) {
		int fd;
		ssize_t n;

		fd = open(entries[i].status_file, O_RDONLY);
		if (fd < 0) {
			perror(entries[i].status_file);
			continue;
		}

		n = read(fd, buf, sizeof(buf) - 1);
		close(fd);

		if (n < 0) {
			perror("read status");
			continue;
		}

		buf[n] = '\0';

		if (strstr(buf, "READY") == NULL)
			continue;

		fd = open(entries[i].boot_count_file, O_RDONLY);
		if (fd < 0) {
			perror(entries[i].boot_count_file);
			continue;
		}

		n = read(fd,
				entries[i].boot_count,
				entries[i].boot_count_size);

		close(fd);

		if (n != (ssize_t)entries[i].boot_count_size) {
			if (n < 0)
				perror("read boot_count");
			else
				fprintf(stderr,
						"%s: invalid size %zd\n",
						entries[i].boot_count_file,
						n);

			continue;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	const char *dev;
	unsigned int cmd = SBI_EXT_TELEMETRY_RPROC_COMMAND;
	struct user_data user_data;
	struct telemetry_data *p_tm_data = NULL;
	char *buffer = NULL;
	int fd = -1;
	int ret = EXIT_FAILURE;

	if (argc < 3 || argc > 4) {
		fprintf(stderr,
				"Usage: %s /dev/scai_tm_rproc <mode> <stop_service>\n"
				"  mode:\n"
				"    0 - concise output\n"
				"    1 - verbose output\n"
				"    2 - stop HSS services\n",
				argv[0]);
		fprintf(stderr,
				"  stop_service(if mode == 2)\n"
				"    1 - stop telemetry publishing via UART @HSS\n"
				"    2 - stop external watchdog pining @HSS\n");
		return EXIT_FAILURE;
	}

	dev = argv[1];
	user_data.arg0 = strtoul(argv[2], NULL, 0);
	if (user_data.arg0 < 0 || user_data.arg0 > 2) {
		fprintf(stderr,
				"Invalid mode: %ld\n"
				"Supported modes:\n"
				"  0 - concise\n"
				"  1 - verbose\n"
				"  2 - stop HSS services\n",
				user_data.arg0);
		return EXIT_FAILURE;
	}

	if (user_data.arg0 == SBI_TM_EXT_STOP_SERVICE) {
		if (argc != 4) {
			fprintf(stderr,
					"No inserted for stop_service list\n"
					"Supported stop services:\n"
					"    1 - stop telemetry publishing via UART @HSS\n"
					"    2 - stop external watchdog pining @HSS\n");
			return EXIT_FAILURE;
		}
		user_data.size = strtol(argv[3], NULL, 0);
		if (user_data.size < 1 || user_data.size > 2) {
			fprintf(stderr,
					"Invalid stop_service list: %ld\n"
					"Supported stop services:\n"
					"    1 - stop telemetry publishing via UART @HSS\n"
					"    2 - stop external watchdog pining @HSS\n",
					user_data.size);
			return EXIT_FAILURE;
		}
	}

	fd = open(dev, O_RDWR);
	if (fd < 0) {
		perror("open");
		goto out;
	}

	buffer = calloc(1, TM_BUFFER_SIZE);
	if (!buffer) {
		perror("calloc");
		goto out;
	}

	if (user_data.arg0 == SBI_TM_EXT_CONCISE ||
			user_data.arg0 == SBI_TM_EXT_VERBOSE) {
		user_data.size = TM_BUFFER_SIZE;
		user_data.buf = buffer;
	}

	printf("Request\n");
	printf("  arg0 : %ld\n", user_data.arg0);
	printf("  size : %ld bytes\n", user_data.size);
	printf("  buf  : %p\n", user_data.buf);

	if (ioctl(fd, cmd, &user_data) < 0) {
		perror("ioctl");
		goto out;
	}

	printf("\nResult\n");
	printf("  size : %ld bytes\n", user_data.size);
	printf("----------------------------------------\n");
	if (user_data.arg0 == SBI_TM_EXT_VERBOSE) {
		printf("%s", buffer);
	} else if (user_data.arg0 == SBI_TM_EXT_CONCISE) {
		p_tm_data = (struct telemetry_data *)user_data.buf;
		set_boot_count(p_tm_data);
		print_tm_data(p_tm_data);
	}
	printf("\n----------------------------------------\n");

	ret = EXIT_SUCCESS;

out:
	if (fd >= 0)
		close(fd);

	free(buffer);

	return ret;
}
