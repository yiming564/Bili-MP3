#ifndef __APP_BASE_H
#define __APP_BASE_H

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern EventGroupHandle_t global_evt;

#ifdef __cplusplus
}
#endif

#endif