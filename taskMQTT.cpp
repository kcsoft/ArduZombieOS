#include <string.h>
#include <Arduino_FreeRTOS.h>

#include "config.h"
#include "taskMQTT.h"
#include "taskManager.h"
#include "taskSettings.h"
#include "taskLights.h"
#include "taskBlink.h"
#include "fast_atoi.h"

QueueHandle_t mqttQueue;
uint32_t notificationMqtt;

// subscribe
char mqttLightTopic[] = "house/\0/light/\0\0\0";
char mqttBlinkTopic[] = "house/\0/blink/\0\0\0";
char mqttStatusTopicAsk[] = "house/\0\0";
char mqttSettingTopic[] = "house/\0/set\0";
char mqttResetTopic[] = "house/\0/reset\0";
// publish
char mqttLightStateTopic[] = "house/\0/light/state/\0\0\0";


void mqttCallback(char* topic, byte* payload, unsigned int length);

EthernetClient client;
PubSubClient mqttClient((char *)MQTT_HOST, 1883, mqttCallback, client);

uint8_t setupTopicForLight(char *topic, uint8_t idPosition, uint8_t light) {
  uint8_t len = idPosition > 0 ? idPosition : strlen(topic);
  if (light >= 10) {
    topic[len] = '1';
    topic[len + 1] = 0x30 + light - 10;
    topic[len + 2] = 0;
  } else {
    topic[len] = 0x30 + light;
    topic[len + 1] = 0;
  }
  return len;
}

void TaskMQTT(void *pvParameters) { // MQTT Client
  (void) pvParameters;
  uint8_t i;
  uint16_t mqttStateTopicVal;

  mqttQueue = xQueueCreate(LIGHTS, sizeof(int8_t));
  DEBUG_PRINTLN("MQTT: Wait DHCP");

  notificationMqtt = 0;
  while ((notificationMqtt & NOTIFY_IP_READY) != NOTIFY_IP_READY) {
    notificationMqtt = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  }

  // prepare topics, replace ID
  uint8_t idPosition = strlen(mqttLightTopic);
  mqttLightTopic[idPosition] = settings.id;
  mqttBlinkTopic[idPosition] = settings.id;
  mqttStatusTopicAsk[idPosition] = settings.id;
  mqttSettingTopic[idPosition] = settings.id;
  mqttLightStateTopic[idPosition] = settings.id;
  mqttResetTopic[idPosition] = settings.id;

  DEBUG_PRINTLN(mqttLightTopic);
  DEBUG_PRINTLN("MQTT: Can connect now!!");

  while (1) {
    if (!mqttClient.connected()) {
      if (mqttClient.connect(settings.hostname)) {
        notifyTasks(NOTIFY_MQTT_CONNECTED);
        DEBUG_PRINTLN("MQTT: Connected");

        // subscribe to lights
        idPosition = 0;
        i = 0;
        while (i <= LIGHTS) {
          idPosition = setupTopicForLight(mqttLightTopic, idPosition, i);
          mqttClient.subscribe((char *)mqttLightTopic);
          mqttLightTopic[idPosition] = 0;
          i++;
        }

        // subscribe to ask lights states
        mqttClient.subscribe((char *)mqttStatusTopicAsk);

        // add settings topic. /house/1/set i1, /house/1/set hMyHostName
        mqttClient.subscribe((char *)mqttSettingTopic);

        // add reset topic /house/1/reset
        mqttClient.subscribe((char *)mqttResetTopic);
      } else {
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
      }
    } else {
      mqttClient.loop(); // maybe faster??
    }
    // check for mqtt publish requests
    // bit 7=state, bit0-4 light
    if (xQueueReceive(mqttQueue, &i, portTICK_PERIOD_MS) == pdPASS) {
      if (mqttClient.connected()) {
        idPosition = setupTopicForLight(mqttLightStateTopic, 0, (i & 0x1F) + 1);
        mqttStateTopicVal = (i & 128) ? '1' : '0'; // uint16 has 0 second byte
        mqttClient.publish(mqttLightStateTopic, (char *)(&mqttStateTopicVal));
        mqttLightStateTopic[idPosition] = 0;
      }
    }
    // vTaskDelay(portTICK_PERIOD_MS);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  uint8_t myTopicLen, channel;
  uint8_t topicLen = strlen(topic);

  DEBUG_PRINT(topic);
  DEBUG_PRINT(" : ");
  DEBUG_PRINTLN((char *)payload);

  // mqttLightTopic
  myTopicLen = strlen(mqttLightTopic);
  if (topicLen > myTopicLen && strncmp(mqttLightTopic, topic, myTopicLen) == 0) {
    if (topicLen - myTopicLen == 1) {
      channel = topic[myTopicLen] - '0';
    } else {
      channel = (topic[myTopicLen] - '0') * 10 + (topic[myTopicLen + 1] - '0');
    }
    if (channel > LIGHTS)
      return;

    if (channel == 0) { // turn all on/off
      // topic value
      myTopicLen = (length == 1 && payload[0] == '1') ? HIGH : LOW;
      while (channel < LIGHTS) {
        setLight(channel, myTopicLen);
        channel++;
      }
    } else { // turn specific channel on/off
      channel--;
      if (length == 1 && (payload[0] == '1' || payload[0] == '0')) {
        setLight(channel, (payload[0] == '1') ? HIGH : LOW);
      } else { // message is a counter > 1
        setLight(channel, fast_atoi((char *)payload, length));
      }
    }

    return;
  }

  // mqttStatusTopicAsk - send all ON lights
  if (strcmp(mqttStatusTopicAsk, topic) == 0) {
    channel = 0;
    while (channel < LIGHTS) {
      if (lightState[channel] == HIGH) {
        myTopicLen = channel | 128;
        xQueueSend(mqttQueue, &myTopicLen, 0);
      }
      channel++;
    }

    return;
  }

  // mqttSettingTopic:
  // iX - set id
  // hHOSTNAME - set hostname
  // l01 - lF1 - set lightPin
  // ? - reset to default settings
  // b0 .. b4 - set blink mode [no eeprom save]
  if (strcmp(mqttSettingTopic, topic) == 0) {
    if (payload[0] == 'b') { // set blink mode
      setBlinkMode(payload[1] - '0');
      return;
    }

    xSemaphoreTake(settingsMutex, portMAX_DELAY);
    if (payload[0] == 'i') { // set id
      settings.id = payload[1];
    } else
    if (payload[0] == 'h') { // set hostname
      length = length >= 20 ? 20 : length;
      strncpy(settings.hostname, (char *)payload + 1, length - 1);
      settings.hostname[length - 1] = 0;
    } else
    if (payload[0] == 'l') { // set lightPin (2 hex digits)
      settings.lightPin[
        payload[1] > '9' ? (payload[1] - 'A' + 10) : (payload[1] - '0')
      ] = payload[2] > '9' ? (payload[2] - 'A' + 10) : (payload[2] - '0');
    } else
    if (payload[0] == '?') { // reset to default
      setDefaultSettings();
    }
    xSemaphoreGive(settingsMutex);
    notifyTasks(NOTIFY_SETTINGS_CHANGED);

    return;
  }

  // reset using watchdog
  if (strcmp(mqttResetTopic, topic) == 0) {
    // wdt_interrupt_reset_enable(portUSE_WDTO); // was removed from arduinofreertos
    // watchdog resets the board
    cli();
    wdt_reset();
    MCUSR = 0;
    WDTCSR = (1 << WDCE) | (1 << WDE);
    WDTCSR = (1 << WDE) | ( 1 << WDP2);
    while (1) continue;
  }
}
