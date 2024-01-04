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
const char* mqttClientName = "TTGO-T-SIM7070G_2";
const char* mqttUser       = "bartw";
const char* mqttPassword   = "blijfteruit";

const char *topicLed       = "GsmClientTest/led";
const char *topicInit      = "GsmClientTest/init";
const char *topicLedStatus = "GsmClientTest/ledStatus";