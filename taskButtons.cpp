#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "config.h"
#include "taskButtons.h"
#include "taskLights.h"
#include "taskBlink.h"
#include "taskMQTT.h"

uint8_t buttonState[BUTTONS / 8]; // the inputs PINA, PINC
uint16_t buttonDebounce[BUTTONS];

uint8_t lightValue;
uint8_t timer1CountReadButtons;
uint8_t readButtonsPortIndex, readButtonsButtonIndex, readButtonsMask;

mqttQueueData buttonQueueData;

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
                lightValue = toggleLight(readButtonsButtonIndex);
                // a button is pressed, add to queue
                buttonQueueData.type = MQTT_LIGHT_STATE;
                buttonQueueData.light = readButtonsButtonIndex;
                buttonQueueData.state = lightValue == HIGH ? 1 : 0;
                xQueueSendFromISR(mqttQueue, &buttonQueueData, 0);
              }
            } else
            // check if long press
            if (buttonDebounce[readButtonsButtonIndex] < DEBOUNCE_LONG) {
              if (++buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE_LONG) { // button long press
                // if turn off, toggle blink
                if (lightState[readButtonsButtonIndex] == LOW) {
                  toggleBlinkEnabledFromISR(readButtonsPortIndex, readButtonsMask);
                }
              }
            } else
            // check if very long press
            if (buttonDebounce[readButtonsButtonIndex] < DEBOUNCE_VERY_LONG) {
              if (++buttonDebounce[readButtonsButtonIndex] >= DEBOUNCE_VERY_LONG) { // button very long press
                // send button number to MQTT
                buttonQueueData.type = MQTT_BUTTON_VERY_LONG;
                // settings.lightPin[readButtonsButtonIndex] is the light number
                buttonQueueData.light = readButtonsButtonIndex;
                buttonQueueData.state = 0;
                xQueueSendFromISR(mqttQueue, &buttonQueueData, 0);
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
