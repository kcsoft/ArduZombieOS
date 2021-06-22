#include <Arduino_FreeRTOS.h>

#include "taskStatus.h"
#include "taskLights.h"
#include "taskSettings.h"
#include "taskDHCP.h"
#include "taskMQTT.h"
#include "taskTCPServer.h"
#include "taskButtons.h"
#include "taskBlink.h"


void setup() {
#ifdef DEBUG
  Serial.begin(115200);
#endif
  xTaskCreate(TaskStatus, "Status", 128, NULL, 0, NULL);
  xTaskCreate(TaskSettings, "Settings", 256, NULL, 0, NULL);
  xTaskCreate(TaskDHCP, "DHCP", 256, NULL, 0, NULL);
  xTaskCreate(TaskMQTT, "MQTT", 256, NULL, 0, NULL);
  xTaskCreate(TaskTCPServer, "TCPServer", 512, NULL, 0, NULL);
  xTaskCreate(TaskLights, "Lights", 256, NULL, 0, NULL);
  // vTaskStartScheduler();

  setStatus(status_start);

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 2000; // 10000*8*0.0625=5ms 2000=1ms
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS11); // 8 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  setBlinkMode(4);

  interrupts();
}

void loop() {}

// every 1ms
ISR(TIMER1_COMPA_vect) {
  TaskButtons();
  TaskBlink();
}
