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
    unsigned char lightPin[LIGHTS];
};

extern Settings settings;

void setDefaultSettings();
void TaskSettings(void *pvParameters);

#endif
