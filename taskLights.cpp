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

uint8_t toggleLight(uint8_t light) {
  lightState[light] = lightState[light] == LOW ? HIGH : LOW;
  lightOnCounter[light] = 0;
  digitalWrite(lightIOPins[settings.lightPin[light]], lightState[light]);
  return lightState[light];
}

void setLight(uint8_t light, uint16_t  state) {
  lightState[light] = (state == 0 ? LOW : HIGH);
  lightOnCounter[light] = (state > 1 ? state : 0);
  digitalWrite(lightIOPins[settings.lightPin[light]], lightState[light]);
}

void TaskLights(void *pvParameters) {
  (void) pvParameters;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t light;

  light = 0;
  while (light < LIGHTS) {
    pinMode(lightIOPins[settings.lightPin[light]], OUTPUT);
    setLight(light, LOW);
    light++;
  }

  while (1) {
    light = 0;

    while (light < LIGHTS) {
      if (lightOnCounter[light] > 0) {
        if (--lightOnCounter[light] == 0) { // turn off light
          digitalWrite(lightIOPins[settings.lightPin[light]], LOW);
          xQueueSend(mqttQueue, &light, 0);
        }
      }
      light++;
    }
    vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_PERIOD_MS);
  }
}
