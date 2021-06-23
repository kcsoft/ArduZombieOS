#ifndef _CONFIG_H
#define _CONFIG_H

// #define DEBUG

#define DEFAULT_ID          '1'
#define DEFAULT_HOSTNAME    "ArduZombie"

// for settings
#define EEPROM_START        100

#define MQTT_HOST           "192.168.100.2"

#define BUTTONS			        16
#define LIGHTS              BUTTONS
#define DEBOUNCE		        3
#define DEBOUNCE_LONG	      255


#ifdef DEBUG
  #define DEBUG_INIT()      Serial.begin(115200)
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_PRINTDEC(x) Serial.print(x, DEC)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_INIT()
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
#endif

#endif
