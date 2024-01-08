#ifndef _TASK_MQTT_H
#define _TASK_MQTT_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "src/Ethernet/Ethernet.h"
#include "src/MQTT/PubSubClient.h"

enum mqttQueueType {
  MQTT_LIGHT_STATE = 1,
  MQTT_BUTTON_VERY_LONG,
};

struct mqttQueueData {
  uint8_t type;
  uint8_t light;
  uint8_t state;
};

extern QueueHandle_t mqttQueue;
extern PubSubClient mqttClient;
extern EthernetClient client;

void TaskMQTT(void *pvParameters);

#endif
