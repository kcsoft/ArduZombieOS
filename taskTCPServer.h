#ifndef _TASK_TCPSERVER_H
#define _TASK_TCPSERVER_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "src/Ethernet/Ethernet.h"

void TaskTCPServer(void *pvParameters);

#endif
