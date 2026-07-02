#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#define TM_BUFFER_SIZE     (16 * 1024 * 1024)

struct user_data {
	long size;
	void *buf;
};

enum {
	SBI_EXT_TELEMETRY_RPROC_COMMAND = 0x14,
};

int main(int argc, char *argv[])
{
	const char *dev;
	unsigned int cmd;
	struct user_data user_data;
	char *buffer = NULL;
	int fd = -1;
	int ret = EXIT_FAILURE;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <device> <cmd>\n", argv[0]);
		return EXIT_FAILURE;
	}

	dev = argv[1];
	cmd = strtoul(argv[2], NULL, 0);

	if (cmd != SBI_EXT_TELEMETRY_RPROC_COMMAND) {
		fprintf(stderr, "Unsupported command: 0x%X\n", cmd);
		return EXIT_FAILURE;
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

	user_data.size = TM_BUFFER_SIZE;
	user_data.buf = buffer;

	printf("Request\n");
	printf("  size : %ld bytes\n", user_data.size);
	printf("  buf  : %p\n", user_data.buf);

	if (ioctl(fd, cmd, &user_data) < 0) {
		perror("ioctl");
		goto out;
	}

	printf("\nResult\n");
	printf("  size : %ld bytes\n", user_data.size);
	printf("----------------------------------------\n");
	printf("%s", buffer);
	printf("\n----------------------------------------\n");

	ret = EXIT_SUCCESS;

out:
	if (fd >= 0)
		close(fd);

	free(buffer);

	return ret;
}
