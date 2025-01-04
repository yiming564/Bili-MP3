#include "app_base.h"
#include "app_file.h"
#include "svc_display.h"
#include <esp_littlefs.h>
#include <sys/stat.h>
#include <jpeg_decoder.h>

static const char app_tag[] = {"app_file"};
const char sys_label[] = {"sys_data"};
static const char avatar[] = {"/sys_data/avatar.jpg"};

void decode_image()
{
	struct stat st;
	stat(avatar, &st);
	FILE *fp = fopen(avatar, "r");
	void *avatar_bin = heap_caps_malloc(st.st_size, MALLOC_CAP_SPIRAM);
	fread(avatar_bin, st.st_size, 1, fp);
	color_t *decode_bin = heap_caps_malloc(128 * 128 * 2, MALLOC_CAP_SPIRAM);
	esp_jpeg_image_cfg_t jpeg_cfg = {
		.indata = avatar_bin,
		.indata_size = st.st_size,
		.outbuf = decode_bin,
		.outbuf_size = 128 * 128 * 2,
		.out_format = JPEG_IMAGE_FORMAT_RGB565,
		.out_scale = JPEG_IMAGE_SCALE_1_2,
	};

	esp_jpeg_image_output_t decode_st;
	esp_jpeg_decode(&jpeg_cfg, &decode_st);

	vTaskDelay(pdMS_TO_TICKS(1000));

	ESP_LOGI(app_tag, "%s: %dx%d.", avatar, decode_st.width, decode_st.height);

	for (uint16_t i = 0; i < 128; i++)
		for (uint16_t j = 0; j < 128; j++)
			PIXEL(i, j) = decode_bin[i + (j << 7)];
	update_fb();

	ESP_LOGI(app_tag, "updated!");

	free(avatar_bin);
	free(decode_bin);
	fclose(fp);
}

void app_file()
{
	WAIT_FOR_START();
	ESP_LOGI(app_tag, "Launched!");
	esp_vfs_littlefs_conf_t lfs_cfg = {
		.base_path = "/sys_data",
		.partition_label = sys_label,
		.format_if_mount_failed = false, 
		.dont_mount = false,
	};
	ESP_ERROR_CHECK(esp_vfs_littlefs_register(&lfs_cfg));
	size_t total, used;
	ESP_ERROR_CHECK(esp_littlefs_info(sys_label, &total, &used));
	ESP_LOGI(app_tag, "total: %d, used: %d", total, used);
	decode_image();
	vTaskDelete(NULL);
}
