#ifndef _TASK_BLINK_H
#define _TASK_BLINK_H

void setBlinkMode(uint8_t mode);
void toggleBlinkEnabledFromISR(uint8_t button);
void TaskBlink();

#endif
