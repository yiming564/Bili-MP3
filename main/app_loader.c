#include "app_base.h"
#include "svc_display.h"
#include "app_manager.h"
#include "svc_file.h"
#include "app_ui.h"

#include <driver/gpio.h>

static const char app_tag[] = {"app_loader"};

void init_power()
{
	gpio_config_t pwr_cfg = {
		.pin_bit_mask = (1ull << GPIO_PWR_EN),
		.mode = GPIO_MODE_OUTPUT
	};
	gpio_config(&pwr_cfg);
	gpio_set_level(GPIO_PWR_EN, 1);
}

void app_main()
{
	gpio_install_isr_service(0);
	init_power();	// Don't say anything, init power first!

	ESP_LOGI(app_tag, "System launched!");
	global_evt = xEventGroupCreate();

	ESP_LOGI(app_tag, "Loading basic apps...");
	xTaskCreate(svc_display,	"svc_display",	DEFAULT_DRV_STACK_SIZE,	NULL,	DEFAULT_SVC_PRIORITY,	NULL);
	xTaskCreate(svc_file,		"svc_file",		DEFAULT_APP_STACK_SIZE,	NULL,	DEFAULT_SVC_PRIORITY,	NULL);
	xTaskCreate(app_manager,	"app_manager",	DEFAULT_APP_STACK_SIZE,	NULL,	DEFAULT_APP_PRIORITY,	NULL);
	// xTaskCreate(app_ui,			"app_ui",		DEFAULT_APP_STACK_SIZE,	NULL,	DEFAULT_APP_PRIORITY,	NULL);

	ESP_LOGI(app_tag, "Sending signals to start apps...");
	xEventGroupSetBits(global_evt, EVT_START);
}
