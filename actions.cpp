#include <Arduino.h>

#include "actions.h"
#include "taskBlink.h"
#include "taskLights.h"
#include "taskMQTT.h"

// called from ISR (on button presses)
void executeAction(uint8_t action, uint8_t param) {
  switch (action) {
    case ACTION_NONE:
      break;
    case ACTION_TOGGLE_LIGHT:
      toggleLight(param, 1);
      break;
    case ACTION_TOGGLE_BLINK:
      toggleBlinkEnabledFromISR(param);
      break;
    case ACTION_MQTT_PUBLISH_1:
    case ACTION_MQTT_PUBLISH_2:
    case ACTION_MQTT_PUBLISH_3:
      sendToMQTTQueue(MQTT_PUBLISH_BUTTON, param, action - ACTION_MQTT_PUBLISH_1 + 1, 1);
      break;
    default:
      break;
  }
}
