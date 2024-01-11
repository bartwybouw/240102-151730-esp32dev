/**************************************************************
 * Bart Wybouw 31/12/2023
 * 
 * This file contains the credentials for 
 *  - Wifi
 *  - GPRS
 *  - MQTT
 *
 **************************************************************/

// Your GPRS credentials, if any
const char apn[]      = "em";
const char gprsUser[] = "";
const char gprsPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "pixerang-unifi";
const char wifiPass[] = "blijfuitonsnetwerk";

// MQTT details
const char* mqttBroker     = "178.116.210.112";
const int   mqttPort       = 18883;
const char* InitDeviceName = "TTGO-T-SIM7070G_XYZ";
char* deviceName = nullptr; // Will be allocated during setup, will contain the name of the system => NEEDS TO BE STORED permanently!!! 
const char* mqttUser       = "bartw";
const char* mqttPassword   = "blijfteruit";

// Need to add these in MQTT enum MqttTopic
const char *topicLed       = "led";
const char *topicInit      = "init";
const char *topicLedStatus = "ledStatus";
const char *topicDeviceName = "deviceName";
const char *topicRefreshTime = "refreshTime";

// MQTT Client
#define DEFAULT_REFRESH_TIME 180
unsigned int refreshTime = DEFAULT_REFRESH_TIME;