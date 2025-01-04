#include "app_base.h"
#include "svc_display.h"
#include "app_manager.h"
#include "app_ui.h"
// #include <map>
// #include <esp_timer.h>

static const char app_tag[] = {"app_ui"};

#define LOWBIT(x)	((x) & (-(x)))

typedef struct 
{
	int16_t s[LCD_H_RES + 1];
} inner_BIT;
inline void inner_add_point(inner_BIT *this, uint16_t u, int16_t x)
{
	for (; u <= LCD_H_RES; u += LOWBIT(u)) this->s[u] += x;
}
void inner_add(inner_BIT *this, uint16_t l, uint16_t r, int16_t x)
{
	inner_add_point(this, r, x), inner_add_point(this, l, -x);
}

typedef struct
{
	inner_BIT s[LCD_V_RES + 1];
} outer_BIT;

EXT_RAM_BSS_ATTR outer_BIT tree;

void app_ui(void *pvParameters)
{
	WAIT_FOR_START();
	ESP_LOGI(app_tag, "Launched!");

	tree.s[0].s[0] = 0;
	
	// vTaskDelay(pdMS_TO_TICKS(1000));

	// ui_window_t rect = {
	// 	0,
	// 	do_rect_paint
	// };
	// winmap.insert({0, rect});

	while (1) vTaskDelay(pdMS_TO_TICKS(1000));
}