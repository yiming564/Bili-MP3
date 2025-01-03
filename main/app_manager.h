#ifndef __APP_H
#define __APP_H

#include <freertos/event_groups.h>

#ifdef __cplusplus
extern "C" {
#endif

#define EVT_START			(1 << 0)
#define WAIT_FOR_START()	xEventGroupWaitBits(global_evt, EVT_START, pdFALSE, pdTRUE, portMAX_DELAY)

void app_manager(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif