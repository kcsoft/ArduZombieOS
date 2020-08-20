
#include <Arduino_FreeRTOS.h>
#include "src/Ethernet/Ethernet.h"
#include "taskDHCP.h"
#include "taskSettings.h"
#include "taskStatus.h"

uint8_t mac[6] = {0x00, 0x08, 0xDC, 0x1D, 0x62, 0x7F};
SemaphoreHandle_t ipSemaphore;

char hostname[20];

void TaskDHCP(void *pvParameters) { // DHCP Client
  (void) pvParameters;
  TickType_t xLastWakeTime;

  xLastWakeTime = xTaskGetTickCount();
  ipSemaphore = xSemaphoreCreateBinary();

  xSemaphoreTake(settingsMutex, portMAX_DELAY);
  xSemaphoreGive(settingsMutex);

#ifdef DEBUG
  Serial.print("Hostname: "); Serial.println(settings.hostname);
#endif

  mac[5] = settings.id;

  if (Ethernet.begin(mac, settings.hostname) == 0) {
    vTaskSuspend(NULL);
  }

#ifdef DEBUG
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }
  Serial.println();
#endif

  setStatus(status_dhcp_connected);
  xSemaphoreGive(ipSemaphore);

  while(1) {
    Ethernet.maintain();
    vTaskDelayUntil( &xLastWakeTime, ( 1000 / portTICK_PERIOD_MS ) ); // 1 second
  }
}
