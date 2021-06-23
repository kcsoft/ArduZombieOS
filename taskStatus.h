#ifndef _TASK_STATUS_H
#define _TASK_STATUS_H

enum boardStatus {
  STATUS_INIT = 1,
  STATUS_DHCP_CONNECTED,
  STATUS_MQTT_CONNECTED
};

void TaskStatus(void *pvParameters);

#endif
