#include "app_base.h"
#include "svc_display.h"
#include "svc_file.h"
#include "app_manager.h"
#include "app_ui.h"
#include <memory.h>
#include <esp_elf.h>
#include <esp_sleep.h>

EventGroupHandle_t global_evt;

static const char app_tag[] = {"app_manager"};

static char buf[1024];
void task_dump()
{
	printf("=====  TASK DUMP BEGIN  =====\n");
	vTaskList(buf);
	printf("Name\t\tState\tPrio\tStack\tNum\tCoreID\n%s\n", buf);
	printf("=====   TASK DUMP END   =====\n");
}
void mem_dump()
{
	printf("===== MEMORY DUMP BEGIN =====\n");
	printf("Heap info for MALLOC_CAP_8BIT:\n");
	heap_caps_print_heap_info(MALLOC_CAP_8BIT);
	printf("Heap info for MALLOC_CAP_SPIRAM:\n");
	heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
	printf("\nHeap info for MALLOC_CAP_INTERNAL:\n");
	heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
	printf("\nHeap info for MALLOC_CAP_DMA:\n");
	heap_caps_print_heap_info(MALLOC_CAP_DMA);
	printf("=====  MEMORY DUMP END  =====\n");
}
void rgb_demo()
{
	int r = 0, g = 1, b = 0;
	color_t c = {0, 0, 0};
	c.r = 31;
	while (1)
	{
		c.r += r, c.g += g, c.b += b;
		if (r == -1 && c.r == 0)
			r = 0, g = 0, b = 1;
		if (g == -1 && c.g == 0)
			r = 1, g = 0, b = 0;
		if (b == -1 && c.b == 0)
			r = 0, g = 1, b = 0;
		if (r == 1 && c.r == 31)
			r = 0, g = 0, b = -1;
		if (g == 1 && c.g == 63)
			r = -1, g = 0, b = 0;
		if (b == 1 && c.b == 31)
			r = 0, g = -1, b = 0;
		for (int i = 0; i < LCD_H_RES * LCD_V_RES; i++) idle_fb[i] = c;
		update_fb();
		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
void light_sleep_test()
{
	ESP_LOGI(app_tag, "Zzz...");
	vTaskDelay(pdMS_TO_TICKS(50));
	esp_sleep_enable_timer_wakeup(portMAX_DELAY);
	esp_light_sleep_start();
	ESP_LOGI(app_tag, "Nice dream!");
}

#include <esp_timer.h>
static int64_t time_bg, time_ed;
static const char wallpaper_cache_path[] = {SYS_PATH "/assets/wallpaper_cache.bin"};
esp_err_t load_cache()
{
	ESP_LOGI(app_tag, "Loading cache...");
	FILE *fp = fopen(wallpaper_cache_path, "rb");
	APP_CHECK_WITH_LOG(fp != NULL, ESP_ERR_NOT_FOUND, app_tag, "Cannot open file: %s", wallpaper_cache_path);

	time_bg = esp_timer_get_time();
	fread(idle_fb, LCD_H_RES * LCD_V_RES * 2, 1, fp);
	time_ed = esp_timer_get_time();
	ESP_LOGI(app_tag, "Total time: %lld ms.", (time_ed - time_bg) / 1000);
	fclose(fp);
	
	update_fb();
	return ESP_OK;
}

void app_manager(void *pvParameters)
{
	WAIT_FOR_START();
	ESP_LOGI(app_tag, "Launched!");

	vTaskDelay(pdMS_TO_TICKS(10000));

	// rgb_demo();
	
	// esp_elf_t test;
	// esp_elf_init(&test);
	// esp_elf_relocate(&test, );
	// esp_elf_request(&test, 0, 0, NULL);
	// esp_elf_deinit(&test);

	while (1)
	{
		load_cache();
		dump_file(EXT_PATH "/English.txt");
		dump_file(EXT_PATH "/中文.txt");
		vTaskDelay(pdMS_TO_TICKS(1000));
		// esp_sleep_enable_timer_wakeup(59000000);
		// esp_light_sleep_start();
		// task_dump();
		// memory_dump();
	}
}
