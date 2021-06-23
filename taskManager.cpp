#include <Arduino_FreeRTOS.h>
#include "config.h"
#include "taskManager.h"

#include "taskStatus.h"
#include "taskSettings.h"
#include "taskDHCP.h"
#include "taskMQTT.h"
#include "taskTCPServer.h"
#include "taskLights.h"

BaseType_t pxHPTaskWoken = 0;

tmTask tmTasks[] = {
  { .pvTaskCode = TaskStatus, .name = "Status", .stack = 128 },
  { .pvTaskCode = TaskSettings, .name = "Settings", .stack = 256 },
  { .pvTaskCode = TaskDHCP, .name = "DHCP", .stack = 256 },
  { .pvTaskCode = TaskMQTT, .name = "MQTT", .stack = 256 },
  // { .pvTaskCode = TaskTCPServer, .name = "TCPServer", .stack = 2048 },
  { .pvTaskCode = TaskLights, .name = "Lights", .stack = 256 },
};

void createTasks() {
  for (uint8_t i = 0; i < sizeof tmTasks / sizeof tmTasks[0]; i++) {
    xTaskCreate(tmTasks[i].pvTaskCode, tmTasks[i].name, tmTasks[i].stack, NULL, 0, &(tmTasks[i].handle));
  }
}

void notifyTasks(uint32_t notification) {
  for (uint8_t i = 0; i < sizeof tmTasks / sizeof tmTasks[0]; i++) {
    xTaskNotify(tmTasks[i].handle, notification, eSetBits);
  }
}

void notifyTasksFromISR(uint32_t notification) {
  for (uint8_t i = 0; i < sizeof tmTasks / sizeof tmTasks[0]; i++) {
    xTaskNotifyFromISR(tmTasks[i].handle, notification, eSetBits, &pxHPTaskWoken);
  }
}
