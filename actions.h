#ifndef _ACTIONS_H
#define _ACTIONS_H

enum actions {
  ACTION_NONE = 0,
  ACTION_TOGGLE_LIGHT, // param = light number
  ACTION_TOGGLE_BLINK, // param = button number
  ACTION_MQTT_PUBLISH_1, // param = button number
  ACTION_MQTT_PUBLISH_2, // param = button number
  ACTION_MQTT_PUBLISH_3 // param = button number
};

void executeAction(uint8_t action, uint8_t param);

#endif
