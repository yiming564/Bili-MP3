#ifndef __SVC_DISPLAY_H
#define __SVC_DISPLAY_H


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	uint16_t b : 5;
	uint16_t g : 6;
	uint16_t r : 5;
} color16_t;
typedef color16_t color_t;

#define COLOR(R, G, B)	((color_t){.r = (R), .g = (G), .b = (B)})

inline color_t alpha_blend(color_t fc, color_t bc, uint8_t alpha)
{
	return COLOR(
		(fc.r * (alpha) + bc.r * (255 - alpha)) / 255,
		(fc.g * (alpha) + bc.g * (255 - alpha)) / 255,
		(fc.b * (alpha) + bc.b * (255 - alpha)) / 255
	);
}

#define EVT_DISP_READY		(1 << 0)
#define EVT_DISP_MODIFIED	(1 << 1)

extern color_t *idle_fb;

#define PIXEL(x, y) (idle_fb[(x) + (y) * LCD_H_RES])

void svc_display(void *pvParameters);
void update_fb();

#ifdef __cplusplus
}
#endif

#endif