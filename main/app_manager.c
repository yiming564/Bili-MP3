#include "app_base.h"
#include "svc_display.h"
#include "app_manager.h"
#include "app_ui.h"
#include <memory.h>
#include <esp_elf.h>
#include <esp_sleep.h>

EventGroupHandle_t global_evt;

static const char app_tag[] = {"app_manager"};

static char buf[1024];
static void task_dump()
{
	vTaskList(buf);
	printf("Name\t\tState\tPrio\tStack\tNum\tCoreID\n%s\n", buf);
}
static void memory_dump()
{
	printf("Heap info for MALLOC_CAP_8BIT:\n");
	heap_caps_print_heap_info(MALLOC_CAP_8BIT);
	printf("Heap info for MALLOC_CAP_SPIRAM:\n");
	heap_caps_print_heap_info(MALLOC_CAP_SPIRAM);
	printf("\nHeap info for MALLOC_CAP_INTERNAL:\n");
	heap_caps_print_heap_info(MALLOC_CAP_INTERNAL);
	printf("\nHeap info for MALLOC_CAP_DMA:\n");
	heap_caps_print_heap_info(MALLOC_CAP_DMA);
}
void rgb_demo()
{
	int r = 0, g = 1, b = 0;
	color_t c; c.raw = 0;
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
void app_manager(void *pvParameters)
{
	WAIT_FOR_START();
	ESP_LOGI(app_tag, "Launched!");

	vTaskDelay(pdMS_TO_TICKS(1000));

	rgb_demo();
	
	// esp_elf_t test;
	// esp_elf_init(&test);
	// esp_elf_relocate(&test, );
	// esp_elf_request(&test, 0, 0, NULL);
	// esp_elf_deinit(&test);

	while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}
