#ifndef _TASK_NOTIFICATION_H
#define _TASK_NOTIFICATION_H

enum {
  NOTIFY_SETTINGS_READY = 1 << 0,
  NOTIFY_SETTINGS_CHANGED = 1 << 1,
  NOTIFY_STATUS_CHANGED = 1 << 2,
  NOTIFY_IP_READY = 1 << 3,
  NOTIFY_MQTT_CONNECTED = 1 << 4,
  NOTIFY_MQTT_DISCONNECTED = 1 << 3, // same as NOTIFY_IP_READY
  NOTIFY_RESET = 1 << 5,
};

struct tmTask {
  TaskFunction_t pvTaskCode;
  const char * const name;
  uint16_t stack;
  TaskHandle_t handle;
};

extern tmTask tmTasks[];

void createTasks();
void notifyTasks(uint32_t notification);
void notifyTasksFromISR(uint32_t notification);

#endif
