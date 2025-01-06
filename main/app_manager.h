#ifndef __APP_H
#define __APP_H

#include <freertos/event_groups.h>

#ifdef __cplusplus
extern "C" {
#endif

void task_dump();
void mem_dump();
void app_manager(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif