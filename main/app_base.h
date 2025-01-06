#ifndef __APP_BASE_H
#define __APP_BASE_H

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_err.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern EventGroupHandle_t global_evt;
#define EVT_START			(1 << 0)
#define WAIT_FOR_START()	xEventGroupWaitBits(global_evt, EVT_START, pdFALSE, pdTRUE, portMAX_DELAY)

#ifdef __cplusplus
}
#endif

#define ESP_CHECK_WITH_LOG(x, error_code, tag, format, ...)			\
	do																\
	{																\
		if (x != ESP_OK) 											\
		{ 															\
			ESP_LOGE(tag, format, ##__VA_ARGS__); 					\
			return error_code; 										\
		} 															\
	} while (0)

#define APP_CHECK_WITH_LOG(cond, error_code, tag, format, ...)		\
	do																\
	{																\
		if (!cond) 													\
		{ 															\
			ESP_LOGE(tag, format, ##__VA_ARGS__); 					\
			return error_code; 										\
		} 															\
	} while (0)

#endif