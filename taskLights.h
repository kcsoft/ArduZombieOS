#ifndef _TASK_LIGHTS_H
#define _TASK_LIGHTS_H

#include "config.h"

extern uint8_t lightState[LIGHTS];
extern uint16_t lightOnCounter[LIGHTS];

uint8_t toggleLight(uint8_t light);
void setLight(uint8_t light, uint16_t state);

void TaskLights(void *pvParameters);

#endif
