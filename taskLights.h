#ifndef _TASK_LIGHTS_H
#define _TASK_LIGHTS_H

#include "config.h"

extern uint8_t lightState[LIGHTS];

void setLight(uint8_t light, uint16_t state, uint8_t isISR);
uint8_t toggleLight(uint8_t light, uint8_t isISR);

void TaskLights(void *pvParameters);

#endif
