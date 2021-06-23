#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "taskManager.h"
#include "taskSettings.h"
#include "taskButtons.h"
#include "taskBlink.h"

void setup() {
  DEBUG_INIT();
  loadSettings();
  setBlinkMode(4);

  createTasks();

  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  OCR1A = 2000; // 10000*8*0.0625=5ms 2000=1ms
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS11); // 8 prescaler
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  interrupts();
}

void loop() {}

// every 1ms
ISR(TIMER1_COMPA_vect) {
  TaskButtons();
  TaskBlink();
}
