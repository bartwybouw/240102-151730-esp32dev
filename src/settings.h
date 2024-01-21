/**************************************************************
 * Bart Wybouw 31/12/2023
 * 
 * This file contains the credentials for 
 *  - Wifi
 *  - GPRS
 *  - MQTT
 *
 **************************************************************/

// SOME GENERAL SETTINGS
// #define     USE_DEEP_SLEEP //Comment this line if you don't want to use deep sleep
#define     TEST_NAME // Set this line if you want to force a name during testing
//#define     SUBSCRIBE_TO_ALL_TOPICS // Comment this line if you don't want to subscribe to all topics

// Your GPRS credentials, if any
const char apn[]      = "em";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "";
const char wifiPass[] = "";

// MQTT details
const char* mqttBroker     = "178.116.210.112";
const int   mqttPort       = 18883;
const char* InitDeviceName = "TTGO-T-SIM7070G_XYZ";
char* deviceName = nullptr;                         // Will be allocated during setup, will contain the name of the system => NEEDS TO BE STORED permanently!!! 
const char* THISdeviceName = "TTGO-T-SIM7070G_1"; // Only use to force a name during testing, combine with >>#define TEST_NAME<<

const char* mqttUser       = "";
const char* mqttPassword   = "";
String baseTopic = "IIOT"; //This is a prefix for all topics, so you can share a single MQTT server between several projects

// Array of topic strings which will be used to subscribe to topics
const char* mqttTopics[] = {
    "init",
    "led",
    "deviceName",
    "updateTime",
    "useDeepSleep",
    "espRestart",
};

const int mqttTopicsCount = sizeof(mqttTopics) / sizeof(mqttTopics[0]);

//* OLD TOPICS PART*/
// Need to add these in MQTT enum MqttTopic

const char *topicLed       = "led";
const char *topicInit      = "init";
const char *topicLedStatus = "ledStatus";
const char *topicDeviceName = "deviceName";
const char *topicUpdateTime = "updateTime";
const char *topicUseDeepSleep = "useDeepSleep";
const char *topicEspRestart = "espRestart";


// MQTT Client
#define DEFAULT_UPDATE_TIME 10 // Time in seconds between updates to the MQTT server
