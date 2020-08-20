#ifndef _TASK_MQTT_H
#define _TASK_MQTT_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "src/Ethernet/Ethernet.h"
#include "src/MQTT/PubSubClient.h"

extern QueueHandle_t mqttQueue;
extern PubSubClient mqttClient;
extern EthernetClient client;

void TaskMQTT(void *pvParameters);

#endif
