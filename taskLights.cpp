#include <Arduino.h>

#include "taskLights.h"
#include "taskMQTT.h"
#include "taskSettings.h"

const uint8_t lightIOPins[LIGHTS] = {
  A8, A9, A10, A11, A12, A13, A14, A15,
  42, 43, 44, 45, 46, 47, 48, 49
};

uint8_t lightState[LIGHTS];
uint16_t lightOnCounter[LIGHTS];

mqttQueueData lightQueueData;

void setLight(uint8_t light, uint16_t  state, uint8_t isISR) {
  lightState[light] = (state == 0 ? LOW : HIGH);
  lightOnCounter[light] = (state > 1 ? state : 0);
  digitalWrite(lightIOPins[light], lightState[light]);
  // publish light state
  lightQueueData.type = MQTT_PUBLISH_LIGHT_STATE;
  lightQueueData.item = light;
  lightQueueData.state = lightState[light];
  if (isISR) {
    xQueueSendFromISR(mqttQueue, &lightQueueData, 0);
  } else {
    xQueueSend(mqttQueue, &lightQueueData, 0);
  }
}

uint8_t toggleLight(uint8_t light, uint8_t isISR) {
  setLight(light, lightState[light] == LOW ? 1 : 0, isISR);
  return lightState[light];
}


void TaskLights(void *pvParameters) {
  (void) pvParameters;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t light;

  light = 0;
  while (light < LIGHTS) { // set lights as output and turn off
    pinMode(lightIOPins[light], OUTPUT);
    digitalWrite(lightIOPins[light], LOW);
    lightState[light] = LOW;
    lightOnCounter[light] = 0;
    light++;
  }

  while (1) {
    light = 0;
    while (light < LIGHTS) {
      if (lightOnCounter[light] > 0) {
        if (--lightOnCounter[light] == 0) { // turn off light
          setLight(light, 0, 0);
        }
      }
      light++;
    }
    vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_PERIOD_MS);
  }
}
