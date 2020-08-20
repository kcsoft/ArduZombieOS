#ifndef _TASK_STATUS_H
#define _TASK_STATUS_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>

extern QueueHandle_t statusQueue;

enum boardStatus {
    status_start = 1,
    status_dhcp_connected,
    status_mqtt_connected
};

void setStatus(boardStatus status);
void setStatusFromISR(boardStatus status);
void TaskStatus(void *pvParameters);

#endif
