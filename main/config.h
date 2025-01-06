#ifndef __CONFIG_H
#define __CONFIG_H

#include <sdkconfig.h>

#define DEFAULT_APP_STACK_SIZE	(4096)
#define DEFAULT_DRV_STACK_SIZE	(4096)
#define DEFAULT_APP_PRIORITY	(1)
#define DEFAULT_SVC_PRIORITY	(2)

#define GPIO_RST		(9)
#define GPIO_CS			(39)
#define GPIO_DC			(38)
#define GPIO_BL			(14)
#define GPIO_SCLK		(12)
#define GPIO_MOSI		(11)
#define GPIO_MISO		(13)

#define GPIO_PWR_EN		(18)

#define GPIO_SDMMC_CD	(4)

#define LEDC_MODE		LEDC_LOW_SPEED_MODE
#define LEDC_TIMER		LEDC_TIMER_0
#define LEDC_CHANNEL	LEDC_CHANNEL_0
#define LEDC_DUTY		LEDC_TIMER_12_BIT
#define LEDC_LIM		(1 << LEDC_DUTY)
#define LEDC_FREQ		(4000)


#define LCD_H_RES		(320)
#define LCD_V_RES		(240)

#define LCD_HOST		SPI2_HOST
#define LCD_FREQ		(80 * 1000 * 1000)
#define POS(x, y)		((x) + (y) * LCD_H_RES)

#define LCD_UPD_HZ		(60)
#define LCD_UPD_PRED_MS	((1000) / LCD_UPD_HZ)

#define SYS_LABEL		"sys_data"
#define SYS_PATH		"/sys"
#define EXT_PATH		"/ext"

#endif