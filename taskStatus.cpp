#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "taskStatus.h"
#include "taskManager.h"

enum boardStatus status;
uint32_t notificationStatus;

void TaskStatus(void *pvParameters) {
  (void) pvParameters;

  uint8_t i;
  pinMode(LED_BUILTIN, OUTPUT);
  status = STATUS_INIT;

  while (1) {
    notificationStatus = ulTaskNotifyTake(pdTRUE, portTICK_PERIOD_MS);
    if (notificationStatus & NOTIFY_IP_READY) {
      status = STATUS_DHCP_CONNECTED;
    }
    if (notificationStatus & NOTIFY_MQTT_CONNECTED) {
      status = STATUS_MQTT_CONNECTED;
    }
    if (notificationStatus & NOTIFY_RESET) { // reset the board
      status = STATUS_INIT;
      cli();
      wdt_reset();
      // disable watchdog interrupt
      MCUSR = 0; // clear system reset flags
      WDTCSR = 0;
      WDTCSR = (1 << WDCE) | (1 << WDE); // Change Enable + SystemReset Enable
      WDTCSR = (1 << WDE) | ( 1 << WDP2); // 250ms
      while (1) continue;
    }

    // blink status led
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
