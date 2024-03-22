#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "taskManager.h"
#include "taskSettings.h"
#include "taskButtons.h"
#include "taskBlink.h"

void prvSetupTimerInterrupt( void ) {
  TIMSK3 = 0; // disable timer compare interrupt
  TCCR3A = 0;
  TCCR3B = 0;
  OCR3A = 30000; // 15000*8*0.0625=15ms
  TCCR3B |= (1 << WGM32); // CTC mode
  TCCR3B |= (1 << CS31); // 8 prescaler
  TIMSK3 |= (1 << OCIE3A); // enable timer compare interrupt
}
void vPortDelay( const uint32_t ms ) {
  if ( ms < portTICK_PERIOD_MS ) {
    delay( (unsigned long) (ms) );
  } else {
    vTaskDelay( (TickType_t) (ms / portTICK_PERIOD_MS) );
    delay( (unsigned long) ( (ms - portTICK_PERIOD_MS) % portTICK_PERIOD_MS ) );
  }
}

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

  wdt_enable(WDTO_250MS);
}

void loop() {}

// every 1ms
ISR(TIMER1_COMPA_vect) {
  wdt_reset();
  TaskButtons();
  TaskBlink();
}
