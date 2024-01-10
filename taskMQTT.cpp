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
char mqttStartTopic[] = "house/\0/start\0";
char mqttLightStateTopic[] = "house/\0/light/state/\0\0\0";
char mqttButtonTopic[] = "house/\0/button/\0\0\0";
char mqttSendSettingTopic[] = "house/\0/settings\0";

mqttQueueData queueData;
// global, used in sendToMQTTQueue
mqttQueueData queueDataSend, queueDataSendISR;

void mqttCallback(char* topic, byte* payload, unsigned int length);

EthernetClient client;
PubSubClient mqttClient((char *)MQTT_HOST, 1883, mqttCallback, client);

uint8_t setupTopicForItem(char *topic, uint8_t idPosition, uint8_t item) {
  uint8_t len = idPosition > 0 ? idPosition : strlen(topic);
  if (item >= 10) {
    topic[len] = '1';
    topic[len + 1] = 0x30 + item - 10;
    topic[len + 2] = 0;
  } else {
    topic[len] = 0x30 + item;
    topic[len + 1] = 0;
  }
  return len;
}

void TaskMQTT(void *pvParameters) { // MQTT Client
  (void) pvParameters;
  uint8_t i;
  uint16_t mqttStateTopicVal;

  mqttQueue = xQueueCreate(LIGHTS, sizeof(mqttQueueData));
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
  mqttButtonTopic[idPosition] = settings.id;
  mqttStartTopic[idPosition] = settings.id;
  mqttSendSettingTopic[idPosition] = settings.id;

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
          idPosition = setupTopicForItem(mqttLightTopic, idPosition, i);
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

        // publish presence on mqttStartTopic
        mqttClient.publish(mqttStartTopic, "1");
      } else {
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
      }
    } else {
      mqttClient.loop(); // maybe faster??
    }
    // check for mqtt publish requests
    if (xQueueReceive(mqttQueue, &queueData, 0) == pdPASS) {
      if (mqttClient.connected()) {
        if (queueData.type == MQTT_PUBLISH_LIGHT_STATE) {
          idPosition = setupTopicForItem(mqttLightStateTopic, 0, queueData.item + 1);
          mqttStateTopicVal = queueData.state ? '1' : '0'; // uint16 has 0 second byte
          mqttClient.publish(mqttLightStateTopic, (char *)(&mqttStateTopicVal));
          mqttLightStateTopic[idPosition] = 0;
        } else
        if (queueData.type == MQTT_PUBLISH_BUTTON) {
          idPosition = setupTopicForItem(mqttButtonTopic, 0, queueData.item + 1);
          mqttStateTopicVal = '0' + queueData.state; // uint16 has 0 second byte
          mqttClient.publish(mqttButtonTopic, (char *)(&mqttStateTopicVal));
          mqttButtonTopic[idPosition] = 0;
        }
      }
    }
    // vTaskDelay(portTICK_PERIOD_MS);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  uint8_t myTopicLen, channel;
  uint8_t button, action, param;
  uint8_t topicLen = strlen(topic);
  mqttQueueData localQueueData;

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
        setLight(channel, myTopicLen, 0); // TODO: check relay no for all setLight calls
        channel++;
      }
    } else { // turn specific channel on/off
      channel--;
      if (length == 1 && (payload[0] == '1' || payload[0] == '0')) {
        setLight(channel, (payload[0] == '1') ? HIGH : LOW, 0);
      } else { // message is a counter > 1
        setLight(channel, fast_atoi((char *)payload, length), 0);
      }
    }

    return;
  }

  // mqttStatusTopicAsk - send all ON lights
  if (strcmp(mqttStatusTopicAsk, topic) == 0) {
    channel = 0;
    while (channel < LIGHTS) {
      if (lightState[channel] == HIGH) {
        localQueueData.type = MQTT_PUBLISH_LIGHT_STATE;
        localQueueData.item = channel;
        localQueueData.state = 1;
        xQueueSend(mqttQueue, &localQueueData, 0);
      }
      channel++;
    }

    return;
  }

  // mqttSettingTopic:
  // iX - set id
  // hHOSTNAME - set hostname
  // asXYZ - set action short for button X (0-F), action Y (0-5), param Z (0-F)
  // amXYZ - set action medium for button X, action Y, param Z
  // alXYZ - set action long for button X, action Y, param Z
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
    if (payload[0] == 'a' // set button actions
      && (payload[1] == 's' || payload[1] == 'm' || payload[1] == 'l') // short, medium or long
      && (payload[2] >= '0' && payload[2] <= 'F') // button number
      && (payload[3] >= '0' && payload[3] <= 'F') // action number
      && (payload[4] >= '0' && payload[4] <= 'F') // param number
    ) {
      button = (payload[2] > '9' ? (payload[2] - 'A' + 10) : (payload[2] - '0')) & 0x0F;
      action = (payload[3] > '9' ? (payload[3] - 'A' + 10) : (payload[3] - '0')) & 0x0F;
      param = (payload[4] > '9' ? (payload[4] - 'A' + 10) : (payload[4] - '0')) & 0x0F;
      if (payload[1] == 's') {
        settings.buttonShortActions[button] = (action << 4) | param;
      } else
      if (payload[1] == 'm') {
        settings.buttonMediumActions[button] = (action << 4) | param;
      } else
      if (payload[1] == 'l') {
        settings.buttonLongActions[button] = (action << 4) | param;
      }
    } else
    if (payload[0] == '?') { // send settings string
      mqttClient.publish(mqttSendSettingTopic, getSettingsString(&payload[1]));
    } else
    if (payload[0] == '0') { // reset to default
      setDefaultSettings();
    }
    xSemaphoreGive(settingsMutex);
    notifyTasks(NOTIFY_SETTINGS_CHANGED);

    return;
  }

  // reset using watchdog, not really working, upon reset FreeRTOS kicks in
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


void sendToMQTTQueue(uint8_t type, uint8_t item, uint8_t state, uint8_t fromISR) {
  if (fromISR) {
    queueDataSendISR.type = type;
    queueDataSendISR.item = item;
    queueDataSendISR.state = state;
    xQueueSendFromISR(mqttQueue, &queueDataSendISR, 0);
  } else {
    queueDataSend.type = type;
    queueDataSend.item = item;
    queueDataSend.state = state;
    xQueueSend(mqttQueue, &queueData, 0);
  }
}
