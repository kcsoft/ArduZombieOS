#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <EEPROM.h>
#include <util/crc16.h>
#include <string.h>

#include "config.h"
#include "taskSettings.h"

SemaphoreHandle_t settingsMutex;
SemaphoreHandle_t settingsSetSemaphore;

Settings settings;

uint16_t calcCRC(uint8_t* buf, uint8_t len) {
  uint16_t crc = 0;
  while (--len)
    crc = _crc16_update(crc, *buf++);
  return crc;
}

void setDefaultSettings() {
  uint8_t i;

  settings.id = DEFAULT_ID;
  strncpy(settings.hostname, DEFAULT_HOSTNAME, 20);
  i = 0;
  while (i < LIGHTS) { // set lightPin to 0,1,2..
    settings.lightPin[i] = i;
    i++;
  }
  i = 0;
  while (i < BUTTONS/8) { // set blinkEnabled to 0,1,2..
    settings.blinkEnabled[i] = 255;
    i++;
  }
}

void TaskSettings(void *pvParameters) {
  (void) pvParameters;

  settingsMutex = xSemaphoreCreateMutex();
  settingsSetSemaphore = xSemaphoreCreateBinary();

  xSemaphoreTake(settingsMutex, portMAX_DELAY);
  EEPROM.get(EEPROM_START, settings);

  // invalid settings, write defaults
  if (calcCRC((uint8_t *)&settings + 2, sizeof(Settings) - 2) != settings.crc) {
#ifdef DEBUG
    Serial.println("Invalid settings");
#endif
    setDefaultSettings();
    settings.crc = calcCRC((uint8_t *)&settings + 2, sizeof(Settings) - 2);
    EEPROM.put(EEPROM_START, settings);
  }

  xSemaphoreGive(settingsMutex);

  while (1) {
    xSemaphoreTake(settingsSetSemaphore, portMAX_DELAY); // settings changed

    xSemaphoreTake(settingsMutex, portMAX_DELAY);
    settings.crc = calcCRC((uint8_t *)&settings + 2, sizeof(Settings) - 2);
    EEPROM.put(EEPROM_START, settings);
    xSemaphoreGive(settingsMutex);
  }
}
