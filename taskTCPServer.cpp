#include <string.h>

#include "src/Ethernet/Ethernet.h"
#include "config.h"
#include "taskTCPServer.h"
#include "taskDHCP.h"
#include "taskSettings.h"

EthernetServer server = EthernetServer(23);
EthernetClient tcpClient;
char recvChar;

void TaskTCPServer(void *pvParameters) {
  (void) pvParameters;

#ifdef DEBUG
  Serial.println("TCPServer: Wait DHCP");
#endif

  if (xSemaphoreTake(ipSemaphore, portMAX_DELAY) != pdPASS) {
    vTaskSuspend(NULL);
  }

#ifdef DEBUG
  Serial.println("TCPServer: Can listen now!!");
#endif

  server.begin();

  xSemaphoreGive(ipSemaphore);

  while (1) {
    tcpClient = server.available();
    if (tcpClient) {
      Serial.print('C');
      while (tcpClient.connected()) {
        if (tcpClient.available()) {
          recvChar = tcpClient.read();
          server.write(recvChar);
#ifdef DEBUG
          Serial.print(recvChar);
#endif
        }
        // vTaskDelay( portTICK_PERIOD_MS );
      }
    }
    // vTaskDelay( portTICK_PERIOD_MS );
  }
}


