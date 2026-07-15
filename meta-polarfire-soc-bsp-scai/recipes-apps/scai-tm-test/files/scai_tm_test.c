#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>

#define TM_BUFFER_SIZE     (16 * 1024 * 1024)

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
};

struct user_data {
	unsigned long arg1;
	long size;
	void *buf;
};

enum {
	SBI_EXT_TELEMETRY_RPROC_COMMAND = 0x14,
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

	if (argc != 3) {
		fprintf(stderr,
				"Usage: %s /dev/scai_tm_rproc <arg1 - concise(0) or verbose(1)>\n",
				argv[0]);
		return EXIT_FAILURE;
	}

	dev = argv[1];
	user_data.arg1 = strtoul(argv[2], NULL, 0);

	if (user_data.arg1 > 1) {
		fprintf(stderr,
				"There is no handler for arg1(%d)\n",
				user_data.arg1);
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
	printf("  arg1 : %ld(%s)\n", user_data.arg1,
			user_data.arg1 == 0 ? (const char *)"concise" :
			(const char*)"verbose");
	printf("  size : %ld bytes\n", user_data.size);
	printf("  buf  : %p\n", user_data.buf);

	if (ioctl(fd, cmd, &user_data) < 0) {
		perror("ioctl");
		goto out;
	}

	printf("\nResult\n");
	printf("  size : %ld bytes\n", user_data.size);
	printf("----------------------------------------\n");
	if (user_data.arg1) {
		printf("%s", buffer);
	} else {
		p_tm_data = (struct telemetry_data *)user_data.buf;
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
