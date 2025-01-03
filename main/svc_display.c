#include "app_base.h"
#include "svc_display.h"
#include "app_manager.h"
#include "app_ui.h"

#include <driver/ledc.h>
#include <driver/spi_master.h>
#include <esp_lcd_io_spi.h>
#include <esp_lcd_panel_st7789.h>
#include <esp_lcd_panel_ops.h>
#include <esp_timer.h>
#include <memory.h>
#include <esp_sleep.h>

#define SWAP(a, b)			\
    do {					\
        typeof(a) temp = a;	\
        a = b;				\
        b = temp;			\
    } while (0)

static const char app_tag[] = {"svc_display"};

EventGroupHandle_t disp_evt;
esp_lcd_panel_handle_t panel_handle = NULL;
static color_t *front_fb = NULL, *back_fb = NULL;
color_t *idle_fb = NULL;

static int64_t time_bg, time_ed;
static bool trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
	BaseType_t isFaWoken = false;
	xEventGroupSetBitsFromISR(disp_evt, EVT_DISP_READY, &isFaWoken);
	portYIELD_FROM_ISR(isFaWoken);
	return false;
}

static void init_backlight()
{
	ESP_LOGI(app_tag, "Init backlight...");
	ledc_fade_func_install(0);
	ledc_timer_config_t ledc_timer_cfg = {
		.speed_mode = LEDC_MODE,
		.duty_resolution = LEDC_DUTY,
		.timer_num = LEDC_TIMER,
		.freq_hz = LEDC_FREQ,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer_cfg));	
	ledc_channel_config_t ledc_channel_cfg = {
		.gpio_num = GPIO_BL,
		.speed_mode = LEDC_MODE,
		.channel = LEDC_CHANNEL,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = LEDC_TIMER,
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel_cfg));
	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_LIM - 1);
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
static void init_lcd()
{
	ESP_LOGI(app_tag, "Init LCD...");
	spi_bus_config_t spi_bus_cfg = {
		.sclk_io_num = GPIO_SCLK,
		.mosi_io_num = GPIO_MOSI,
		.miso_io_num = GPIO_MISO,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = LCD_H_RES * LCD_V_RES * sizeof(color_t),
	};
	ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &spi_bus_cfg, SPI_DMA_CH_AUTO));
	esp_lcd_panel_io_handle_t io_handle = NULL;
	esp_lcd_panel_io_spi_config_t io_cfg = {
		.cs_gpio_num = GPIO_CS,
		.dc_gpio_num = GPIO_DC,
		.pclk_hz = LCD_FREQ,
		.lcd_cmd_bits = 8,
		.lcd_param_bits = 8,
		.spi_mode = 0,
		.trans_queue_depth = 16,
		.on_color_trans_done = trans_done,
	};
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(LCD_HOST, &io_cfg, &io_handle));
	esp_lcd_panel_dev_config_t panel_cfg = {
		.reset_gpio_num = GPIO_RST,
		.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR,
		.bits_per_pixel = 16,
		.data_endian = LCD_RGB_DATA_ENDIAN_LITTLE
	};
	ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_cfg, &panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
	xEventGroupSetBits(disp_evt = xEventGroupCreate(), EVT_DISP_READY);
}

void update_fb() { SWAP(back_fb, idle_fb); xEventGroupSetBits(disp_evt, EVT_DISP_MODIFIED); }
void drv_display(void *pvParameters)
{
	WAIT_FOR_START();
	ESP_LOGI(app_tag, "Launched!");

	init_backlight();
	init_lcd();

	ESP_LOGI(app_tag, "Init frame buffer...");
	front_fb = heap_caps_calloc(LCD_H_RES * LCD_V_RES, sizeof(color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
	back_fb =  heap_caps_calloc(LCD_H_RES * LCD_V_RES, sizeof(color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
	idle_fb =  heap_caps_calloc(LCD_H_RES * LCD_V_RES, sizeof(color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
	update_fb();

	while (xEventGroupWaitBits(disp_evt, EVT_DISP_READY | EVT_DISP_MODIFIED, pdTRUE, pdTRUE, portMAX_DELAY))
	{
		SWAP(front_fb, back_fb);
		ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LCD_H_RES, LCD_V_RES, front_fb));
	}
}