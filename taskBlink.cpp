#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "taskSettings.h"
#include "taskBlink.h"

#define BLINK_MASKS_SIZE	12

const uint8_t blinkMasks[] = {0, 1, 3, 3, 7, 15, 31, 15, 7, 3, 3, 1};
// 16-1 24-3 28-7 30-15 31-31
const uint8_t blinkMasksReverse[] = {0, 16, 0, 24, 0, 0, 0, 28, 0, 0, 0, 0, 0, 0, 0, 30};

uint8_t blinkValue = 0;
uint8_t blinkBitMask = 0;
// half of port is dephased by 0-31 diff in masks
uint8_t blinkKeepCounter = 0;
uint8_t blinkMaskIndex1 = 0;
uint8_t blinkMaskIndex2 = 6;
uint8_t blinkMask1 = 0;
uint8_t blinkMask2 = 0;

struct BlinkMode {
  uint8_t blinkMasks[BLINK_MASKS_SIZE];
  uint8_t blinkStart1;
  uint8_t blinkStart2;
  uint8_t blinkMaxCounter;
};
 // blinkmode 0=off, 4=max blink light
BlinkMode blinkModes[] = {
  {
    blinkMasks: {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    blinkStart1: 0,
    blinkStart2: 1,
    blinkMaxCounter: BLINK_MASKS_SIZE + 28
  },
  {
    blinkMasks: {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    blinkStart1: 0,
    blinkStart2: 1,
    blinkMaxCounter: BLINK_MASKS_SIZE + 28
  },
  {
    blinkMasks: {0, 1, 3, 3, 1, 0, 0, 0, 0, 0, 0, 0},
    blinkStart1: 0,
    blinkStart2: 2,
    blinkMaxCounter: BLINK_MASKS_SIZE + 18
  },
  {
    blinkMasks: {0, 1, 3, 3, 7, 3, 3, 1, 0, 0, 0, 0},
    blinkStart1: 0,
    blinkStart2: 4,
    blinkMaxCounter: BLINK_MASKS_SIZE + 18
  },
  {
    blinkMasks: {0, 1, 3, 3, 7, 15, 31, 15, 7, 3, 3, 1},
    blinkStart1: 0,
    blinkStart2: 6, // half of port is dephased by 0-31 diff in masks
    blinkMaxCounter: BLINK_MASKS_SIZE + 20
  },
};

BlinkMode *blinkMode;

void setBlinkMode(uint8_t mode) {
  if (mode >= (sizeof(blinkModes) / sizeof(blinkModes[0]))) {
    return;
  }
  noInterrupts();
  blinkMode = &blinkModes[mode];
  blinkMaskIndex1 = blinkMode->blinkStart1;
  blinkMaskIndex2 = blinkMode->blinkStart2;
  interrupts();
}

void toggleBlinkEnabled(uint8_t port, uint8_t mask) {
  xSemaphoreTake(settingsMutex, portMAX_DELAY);

  if (settings.blinkEnabled[port] & mask) {
    settings.blinkEnabled[port] &= ~mask;
  } else {
    settings.blinkEnabled[port] |= mask;
  }

  xSemaphoreGive(settingsMutex);
  xSemaphoreGive(settingsSetSemaphore);
}

// called from T1 ISR every 1ms
void TaskBlink() {
  DDRA = blinkValue & settings.blinkEnabled[0];
  DDRC = blinkValue & settings.blinkEnabled[1];

  // compute the value for button blink !! max 4 outputs on per port
  if (blinkBitMask == 0) {
    blinkBitMask = 1 << 5; // use 5 bits
    if (++blinkKeepCounter >= 10) { // 20 * (5 * 5) = 500ms
      blinkKeepCounter = 0;
      if (++blinkMaskIndex1 > blinkMode->blinkMaxCounter)
        blinkMaskIndex1 = 0;
      blinkMask1 = blinkMaskIndex1 >= BLINK_MASKS_SIZE ? 0 : blinkMode->blinkMasks[blinkMaskIndex1];

      if (++blinkMaskIndex2 > blinkMode->blinkMaxCounter)
        blinkMaskIndex2 = 0;
      blinkMask2 = blinkMaskIndex2 >= BLINK_MASKS_SIZE ? 0 : blinkMode->blinkMasks[blinkMaskIndex2];

      if (blinkMask2 < sizeof(blinkMasksReverse))
        blinkMask2 = blinkMasksReverse[blinkMask2];
    }
  } else {
    blinkBitMask >>= 1;
  }

  blinkValue = (blinkMask1 & blinkBitMask ? 0x0F : 0) | (blinkMask2 & blinkBitMask ? 0xF0 : 0);
}
