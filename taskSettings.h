#ifndef _TASK_SETTINGS_H
#define _TASK_SETTINGS_H

#include <semphr.h>
#include "config.h"

extern SemaphoreHandle_t settingsMutex;
extern SemaphoreHandle_t settingsSetSemaphore;

struct Settings {
  uint16_t crc;
  char id;
  char hostname[20];
  unsigned char blinkEnabled[BUTTONS / 8];
  // actions are 8 bit, 4H = actionType, 4L = param (light no or button no)
  unsigned char buttonShortActions[BUTTONS];
  unsigned char buttonMediumActions[BUTTONS];
  unsigned char buttonLongActions[BUTTONS];
};

extern Settings settings;

void setDefaultSettings();
void loadSettings();
void saveSettings();
void TaskSettings(void *pvParameters);
unsigned char *getSettingsString(unsigned char *settingField);

#endif
