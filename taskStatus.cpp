#include <Arduino.h>
#include "taskStatus.h"

QueueHandle_t statusQueue;

void setStatusFromISR(boardStatus status) {
  boardStatus stat = status;
  xQueueSendFromISR(statusQueue, &status, 0);
}

void setStatus(boardStatus status) {
  boardStatus stat = status;
  xQueueSend(statusQueue, &status, 0);
}

void TaskStatus(void *pvParameters) {
  (void) pvParameters;
    enum boardStatus stat, status;
    uint8_t i;
    statusQueue = xQueueCreate(5, sizeof(int8_t));

  pinMode(LED_BUILTIN, OUTPUT);

  while (true) {
    if (xQueueReceive(statusQueue, &stat, portTICK_PERIOD_MS) == pdPASS) {
        status = stat;
    }

    i = 0;
    while (++i <= status) {
        digitalWrite(LED_BUILTIN, HIGH);
        vTaskDelay( 60 / portTICK_PERIOD_MS );
        digitalWrite(LED_BUILTIN, LOW);
        vTaskDelay( 200 / portTICK_PERIOD_MS );
    }
    vTaskDelay( 500 / portTICK_PERIOD_MS );
  }
}
