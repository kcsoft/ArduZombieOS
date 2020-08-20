#ifndef _TASK_DHCP_H
#define _TASK_DHCP_H

#include <semphr.h>

extern SemaphoreHandle_t ipSemaphore;

void TaskDHCP(void *pvParameters);

#endif
