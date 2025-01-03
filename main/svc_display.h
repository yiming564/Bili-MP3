#ifndef __LCD_H
#define __LCD_H


#ifdef __cplusplus
extern "C" {
#endif

typedef union
{
	struct
	{
		uint16_t r : 5;
		uint16_t g : 6;
		uint16_t b : 5;
	};
	uint16_t raw;
} color16_t;
typedef color16_t color_t;

#define EVT_DISP_READY		(1 << 0)
#define EVT_DISP_MODIFIED	(1 << 1)

extern EventGroupHandle_t disp_evt;
extern color_t *idle_fb;

void drv_display(void *pvParameters);
void update_fb();

#ifdef __cplusplus
}
#endif

#endif