// #include "app_base.h"
// #include "drv_display.h"
// #include "app_manager.h"
// #include "app_ui.h"
// #include <map>
// #include <esp_timer.h>

// static const char app_tag[] = {"app_ui"};
// std::map<uint16_t, ui_window_t> winmap;

// static void do_rect_paint(uint32_t pos, pixel_t *fb)
// {
// 	uint16_t xb = pos, yb = pos >> 16, c = esp_timer_get_time();
// 	for (int y = yb; y < yb + LCD_V_RES; y++)
// 		for (int x = xb; x < xb + LCD_H_RES; x++)
// 			fb[POS(x, y)] = c;
// }

// void app_ui(void *pvParameters)
// {
// 	WAIT_FOR_START();
// 	ESP_LOGI(app_tag, "Launched!");
	
// 	vTaskDelay(pdMS_TO_TICKS(1000));

// 	ui_window_t rect = {
// 		0,
// 		do_rect_paint
// 	};
// 	winmap.insert({0, rect});

// 	// while (1)
// 	// {
// 	// 	DISP_LOCK();
// 	// 	for (auto &&[level, window] : winmap) window.callback(window.pos, VRAM);
// 	// 	DISP_UNLOCK();
// 	// }
// }
