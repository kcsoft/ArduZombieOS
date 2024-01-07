#ifndef _TASK_SETTINGS_H
#define _TASK_SETTINGS_H

#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include "config.h"

extern SemaphoreHandle_t settingsMutex;
extern SemaphoreHandle_t settingsSetSemaphore;

struct Settings {
    uint16_t crc;
    char id;
    char hostname[20];
    unsigned char lightPin[LIGHTS];
    uint8_t blinkEnabled[BUTTONS/8];
};

extern Settings settings;

void setDefaultSettings();
void TaskSettings(void *pvParameters);

#endif
