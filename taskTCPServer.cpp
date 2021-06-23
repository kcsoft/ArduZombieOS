#include <string.h>

#include "src/Ethernet/Ethernet.h"
#include "config.h"
#include "taskTCPServer.h"
#include "taskManager.h"

EthernetServer server = EthernetServer(23);
EthernetClient tcpClient;
char recvChar;
uint32_t notificationTcpServer;
boolean alreadyConnected = false;

void TaskTCPServer(void *pvParameters) {
  (void) pvParameters;

  DEBUG_PRINTLN("TCPServer: Wait DHCP");
  notificationTcpServer = 0;
  while ((notificationTcpServer & NOTIFY_IP_READY) != NOTIFY_IP_READY) {
    notificationTcpServer = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }
  DEBUG_PRINTLN("TCPServer: Can listen now!!");

  server.begin();

  while (1) {
    tcpClient = server.available();
    if (tcpClient) {
      if (!alreadyConnected) {
        alreadyConnected = true;
        tcpClient.flush();
        DEBUG_PRINTLN("New client");
      }
      taskENTER_CRITICAL();
      if (tcpClient.available() > 0) {
        recvChar = tcpClient.read();
        DEBUG_PRINT(recvChar);
      }
      taskEXIT_CRITICAL();
      vTaskDelay( 1 );
    }
    vTaskDelay( 1 );
  }
}


