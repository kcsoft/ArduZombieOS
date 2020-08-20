#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "config.h"
#include "taskButtons.h"
#include "taskLights.h"
#include "taskBlink.h"
#include "taskMQTT.h"

uint8_t buttonState[BUTTONS / 8]; // the inputs PINA, PINC
uint8_t buttonDebounce[BUTTONS];

uint8_t lightValue;
uint8_t timer1CountReadButtons;
uint8_t readButtonsPortIndex, readButtonsButtonIndex, readButtonsMask;

// called from T1 ISR every 1ms
void TaskButtons() {
  if (++timer1CountReadButtons >= 5) {
    if (timer1CountReadButtons == 5) { // = 5 aquire inputs
      DDRA = 0;
      DDRC = 0;
      PORTA = 0xFF;
      PORTC = 0xFF;
      delayMicroseconds(40);

      readButtonsPortIndex = 0;
      readButtonsButtonIndex = 0;
      buttonState[0] = PINA;
      buttonState[1] = PINC;
      PORTA = 0;
      PORTC = 0;
    } else { // timer1CountReadButtons > 5, process inputs
      timer1CountReadButtons = 0;
      while (readButtonsPortIndex < (BUTTONS / 8)) {
        readButtonsMask = 128;
        while (readButtonsMask) {
          if ((buttonState[readButtonsPortIndex] & readButtonsMask) == LOW) { // button pressed
            if (buttonDebounce[readButtonsButtonIndex] < DEBOUNCE) {
              if (++buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE) { // button push event
                // a button is pressed, add to queue
                lightValue = toggleLight(readButtonsButtonIndex);
                lightValue = readButtonsButtonIndex | (lightValue == HIGH ? 128 : 0);
                xQueueSendFromISR(mqttQueue, &lightValue, 0);
              }
            } else
            // check if long press
            if (buttonDebounce[readButtonsButtonIndex] < DEBOUNCE_LONG) {
              if (++buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE_LONG) { // button long press
                // if turn off, toggle blink
                if (lightState[readButtonsButtonIndex] == LOW) {
                  toggleBlinkEnabled(readButtonsPortIndex, readButtonsMask);
                }
              }
            }
          } else {
            buttonDebounce[readButtonsButtonIndex] = 0;
          }
          readButtonsButtonIndex++;
          readButtonsMask >>= 1;
        }
        readButtonsPortIndex++;
      }
    }
  }
}
