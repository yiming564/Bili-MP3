#include "app_base.h"
#include "app_file.h"
#include <esp_littlefs.h>
#include <sys/stat.h>

static const char app_tag[] = {"app_file"};
const char sys_label[] = {"sys_data"};
static const char demo_file[] = {"/sys_data/LongFileName.txt"};

static char raw[256];

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

	struct stat st;
	stat(demo_file, &st);
	FILE *fp = fopen(demo_file, "r");

	fread(raw, st.st_size, 1, fp);
	printf(raw);

	fclose(fp);

	vTaskDelete(NULL);
}
