#ifndef _TASK_MQTT_H
#define _TASK_MQTT_H

#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "src/Ethernet/Ethernet.h"
#include "src/MQTT/PubSubClient.h"

enum mqttQueueType {
  MQTT_PUBLISH_LIGHT_STATE = 1, // publish a light state topic
  MQTT_PUBLISH_BUTTON, // publish a button press topic
};

struct mqttQueueData {
  uint8_t type; // mqttQueueType
  uint8_t item;
  uint8_t state;
};

extern QueueHandle_t mqttQueue;
extern PubSubClient mqttClient;
extern EthernetClient client;

void TaskMQTT(void *pvParameters);
void sendToMQTTQueue(uint8_t type, uint8_t item, uint8_t state, uint8_t fromISR);

#endif
