
#include <Arduino_FreeRTOS.h>
#include "src/Ethernet/Ethernet.h"
#include "taskDHCP.h"
#include "taskSettings.h"
#include "taskStatus.h"
#include "taskManager.h"

uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x1D, 0x62, 0x7F};
uint32_t notificationDHCP;

void TaskDHCP(void *pvParameters) { // DHCP Client
  (void) pvParameters;
  TickType_t xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();

  notificationDHCP = 0;
  while ((notificationDHCP & NOTIFY_SETTINGS_READY) != NOTIFY_SETTINGS_READY) {
    notificationDHCP = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }

  DEBUG_PRINT("Hostname: ");
  DEBUG_PRINTLN(settings.hostname);

  mac[5] = settings.id;

  if (Ethernet.begin(mac, settings.hostname) == 0) {
    vTaskSuspend(NULL);
  }

  DEBUG_PRINT("My IP address: ");
  for (uint8_t i = 0; i < 4; i++) {
    DEBUG_PRINTDEC(Ethernet.localIP()[i]);
    DEBUG_PRINT('.');
  }
  DEBUG_PRINTLN();

  notifyTasks(NOTIFY_IP_READY);

  while(1) {
    Ethernet.maintain();
    vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_PERIOD_MS ) ); // 1 second
  }
}
