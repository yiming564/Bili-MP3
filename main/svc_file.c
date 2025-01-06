#include "app_base.h"
#include "svc_display.h"
#include "svc_file.h"
#include "app_manager.h"

#include <esp_littlefs.h>
#include <sys/stat.h>
#include <esp_timer.h>
#include <stb_truetype.h>
#include <jpeg_decoder.h>

#include <driver/sdmmc_host.h>

EventGroupHandle_t file_evt;
uint8_t ext_status = 0;

static const char app_tag[] = {"app_file"};
static const char font_path[] = {SYS_PATH "/simhei.ttf"};
static const char wallpaper_path[] = {SYS_PATH "/wallpaper.jpg"};
static const char wallpaper_cache_path[] = {SYS_PATH "/assets/wallpaper_cache.bin"};

static int64_t time_bg, time_ed;
esp_err_t demo_decode_image()
{
	struct stat st;
	stat(wallpaper_path, &st);
	FILE *fp = fopen(wallpaper_path, "rb");
	APP_CHECK_WITH_LOG(fp != NULL, ESP_ERR_NOT_FOUND, app_tag, "Cannot open file: %s", wallpaper_path);
	void *avatar = heap_caps_malloc(st.st_size, MALLOC_CAP_SPIRAM);
	APP_CHECK_WITH_LOG(avatar != NULL, ESP_ERR_NO_MEM, app_tag, "Out of memory");
	fread(avatar, st.st_size, 1, fp), fclose(fp);
	color_t *raw = heap_caps_malloc(LCD_H_RES * LCD_V_RES * 2, MALLOC_CAP_SPIRAM);
	APP_CHECK_WITH_LOG(raw != NULL, ESP_ERR_NO_MEM, app_tag, "Out of memory");
	esp_jpeg_image_cfg_t jpeg_cfg = {
		.indata = avatar,
		.indata_size = st.st_size,
		.outbuf = (void *)raw,
		.outbuf_size = LCD_H_RES * LCD_V_RES * 2,
		.out_format = JPEG_IMAGE_FORMAT_RGB565,
		.out_scale = JPEG_IMAGE_SCALE_0,
	};

	esp_jpeg_image_output_t decode_st;

	time_bg = esp_timer_get_time();
	ESP_CHECK_WITH_LOG(esp_jpeg_decode(&jpeg_cfg, &decode_st), ESP_FAIL, app_tag, "Failed to decode jpeg");
	time_ed = esp_timer_get_time();

	ESP_LOGI(app_tag, "%s: %dx%d, Total time: %lld ms.", wallpaper_path, decode_st.width, decode_st.height, (time_ed - time_bg) / 1000);
	free(avatar);

	ESP_LOGI(app_tag, "Generating cache...");
	fp = fopen(wallpaper_cache_path, "wb");
	fwrite(raw, LCD_H_RES * LCD_V_RES * 2, 1, fp), fclose(fp);
	free(raw);
	return ESP_OK;
}

extern uint8_t font_data[];
void decode_font()
{
	ESP_LOGI(app_tag, "Decode font...");

	stbtt_fontinfo font_info;

	struct stat st;
	stat(font_path, &st);
	FILE *fp = fopen(font_path, "rb");
	void *font = heap_caps_malloc(st.st_size, MALLOC_CAP_SPIRAM);
	fread(font, st.st_size, 1, fp);
	stbtt_InitFont(&font_info, font, 0), free(font);

	uint8_t *raw = NULL; int w, h;
	raw = stbtt_GetCodepointBitmap(&font_info, 0, stbtt_ScaleForPixelHeight(&font_info, 24), L'姚', &w, &h, 0, 0);

	for (uint16_t x = 0; x < w; x++)
		for (uint16_t y = 0; y < h; y++)
			PIXEL(x, y) = alpha_blend(COLOR(31, 0, 0), PIXEL(x, y), raw[x + y * w]);
	free(raw);
	update_fb();
	ESP_LOGI(app_tag, "Updated! %dx%d, %ld KiB", w, h, st.st_size >> 10);
	fclose(fp);
}

esp_err_t dump_file(const char *path)
{
	struct stat st; stat(path, &st);
	FILE *fp = fopen(path, "r");
	APP_CHECK_WITH_LOG(fp != NULL, ESP_ERR_NOT_FOUND, app_tag, "Cannot open file: %s", path);
	char *buf = heap_caps_malloc(st.st_size, MALLOC_CAP_SPIRAM);
	APP_CHECK_WITH_LOG(buf != NULL, ESP_ERR_NO_MEM, app_tag, "Out of memory");
	fread(buf, st.st_size, 1, fp), fclose(fp);
	printf("%s:\n", path);
	for (int i = 0; i < st.st_size; i++) putchar(buf[i]);
	free(buf);
	printf("=== EOF ===\n");
	return ESP_OK;
}

static esp_err_t init_sys_partition()
{
	ESP_LOGI(app_tag, "Init %s...", SYS_PATH);
	const esp_partition_t *sys_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_LITTLEFS, SYS_LABEL);
	assert(sys_partition != NULL);
	esp_vfs_littlefs_conf_t flash_cfg = {
		.base_path = SYS_PATH,
		.partition = sys_partition,
		.format_if_mount_failed = false, 
		.dont_mount = false,
	};
	return esp_vfs_littlefs_register(&flash_cfg);
}

static sdmmc_card_t card;
static sdmmc_host_t host = SDMMC_HOST_DEFAULT();
static sdmmc_slot_config_t slot_config = {
	.clk = 7,
	.cmd = 15,
	.d0 = 6,
	.d1 = 5,
	.d2 = 17,
	.d3 = 16,
	.cd = -1,	// ESP-IDF will auto deinit CD pin if we register it in slot config, 太 TM 鸡肋了
	.wp = -1,
	.width = 4
};
static esp_vfs_littlefs_conf_t sdmmc_cfg = {
	.base_path = EXT_PATH,
	.sdcard = &card,
	.format_if_mount_failed = false, 
	.dont_mount = false,
};

static esp_err_t init_ext_partition()
{
	if (ext_status == 1) return ESP_OK;
	ESP_LOGI(app_tag, "Init %s...", EXT_PATH);
	host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
	host.flags = SDMMC_HOST_FLAG_4BIT;
	ESP_CHECK_WITH_LOG(host.init(),												ESP_FAIL, app_tag, "SDMMC host init failed!");
	ESP_CHECK_WITH_LOG(sdmmc_host_init_slot(SDMMC_HOST_SLOT_1, &slot_config),	ESP_FAIL, app_tag, "SDMMC slot init failed!");
	ESP_CHECK_WITH_LOG(sdmmc_card_init(&host, &card),							ESP_FAIL, app_tag, "SDMMC card init failed!");
	ESP_CHECK_WITH_LOG(esp_vfs_littlefs_register(&sdmmc_cfg),					ESP_FAIL, app_tag, "LittleFS register failed!");
	ext_status = 1;
	return ESP_OK;
}
static esp_err_t deinit_ext_partition()
{
	if (ext_status == 0) return ESP_OK;
	ESP_LOGI(app_tag, "Deinit %s...", EXT_PATH);
	ESP_CHECK_WITH_LOG(esp_vfs_littlefs_unregister_sdmmc(&card),	ESP_FAIL, app_tag, "LittleFS unregister failed!");
	ESP_CHECK_WITH_LOG(sdmmc_host_deinit(), 						ESP_FAIL, app_tag, "SDMMC host deinit failed!");
	ext_status = 0;
	return ESP_OK;
}

static IRAM_ATTR void do_card_detect(void *arg)
{
	BaseType_t isFaWoken = false;
	xEventGroupSetBitsFromISR(file_evt, EVT_CARD_DETECT, &isFaWoken);
	portYIELD_FROM_ISR(isFaWoken);
}

static void init_sd_card_detect()
{
	gpio_config_t card_detect_cfg = {
		.pin_bit_mask = 1 << GPIO_SDMMC_CD,
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = true,
		.intr_type = GPIO_INTR_ANYEDGE,
	};
	gpio_config(&card_detect_cfg);
	gpio_intr_enable(GPIO_SDMMC_CD);
	gpio_isr_handler_add(GPIO_SDMMC_CD, do_card_detect, NULL);
	xEventGroupSetBits(file_evt = xEventGroupCreate(), EVT_CARD_DETECT);
}

void svc_file()
{
	ESP_LOGI(app_tag, "Launched!");

	init_sys_partition();
	init_sd_card_detect();

	while (xEventGroupWaitBits(file_evt, EVT_CARD_DETECT, pdTRUE, pdTRUE, portMAX_DELAY))
		if (gpio_get_level(GPIO_SDMMC_CD) == ext_status)
		{
			vTaskDelay(pdMS_TO_TICKS(20));
			if (gpio_get_level(GPIO_SDMMC_CD) == ext_status)	// Card status really changed!
				ext_status ? deinit_ext_partition() : init_ext_partition();
		}
}
