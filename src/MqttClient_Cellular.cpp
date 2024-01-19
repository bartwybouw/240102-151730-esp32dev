/**************************************************************
*
* Test client developed for demonstration
* HW ESP32 + GSM Module + SD-card reader, battery, GPS
* Lilygo TTGO T-SIM7070G
* 
* See ReadMe.h for additional info
*  
* Bart Wybouw 29/12/2023 with the support of ChatGPT 4.0
*
**************************************************************/
// Define DEBUG mode with extra output to serial monitor

#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)  Serial.println(x) // Development mode
#else
#define DEBUG_PRINT(x)               // Production mode
#endif

#define TINY_GSM_MODEM_SIM7070       // Define modem type
#define SerialMon Serial             // Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialAT Serial1             // Set serial for AT commands (to the module), use Hardware Serial on Mega, Leonardo, Micro
#define TINY_GSM_DEBUG SerialMon     // Define the serial console for debug prints, if needed

//#define DUMP_AT_COMMANDS // See all AT commands, if wanted

// Standard libraries to include
#include <Arduino.h>                // Needed for PlatformIO C++ compatibility
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <SPI.h>
#include <SD.h>
#include <esp_adc_cal.h>
#include "nvs_flash.h"
#include <Preferences.h>
#include <time.h>

// Include own files
#include "ReadMe.h"                 // Contains additional info on working of the sketch
#include "credentials.h"            // File with credentials
#include "version.h"                // Version info

Ticker tick;

#define uS_TO_S_FACTOR 1000000ULL   // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60           // Time ESP32 will go to sleep (in seconds)
#define LED_PIN 12                  // Blue LED on the ESP32 module, used to indicate GPS lock 
#define KEEP_ALIVE_TIME 10          // Keep alive time for controller before going to deep sleep

// Some global definitions
int ledStatus = LOW;                // Indicates the status of the blue LED
int vref = 1100;                    // For battery measurement
float currentBatteryVoltage = 9.99; // For battery measurement
uint32_t lastReconnectAttempt = 0;
time_t now;
struct tm timeinfo;
String fullTopic;                  // Used to temporarely store the full topic name before using an mqtt.publish
Preferences preferences;            // Used to store device name in NV RAM


#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif

// Create global modem and mqtt instances
TinyGsmClient client(modem);
PubSubClient  mqtt(client);

// *** Include additional files
#include "SDcard_Log_functions.h"   // All SDcard, storage and storage management and log funtions
#include "Modem_functions.h"        // All modem related functions and definitions 
#include "GPS_functions.h"          // All GPS related functions and definitions
#include "MQTT_functions.h"         // All MQTT
#include "general_functions.h"      // General functions



//****** SETUP ******//
void setup()
{
    // Set console baud rate
    Serial.begin(115200);
    Serial.println("Starting...");
    Serial.println("Version: " + String(PROJECT_VERSION_MAJOR) + "." + String(PROJECT_VERSION_MINOR) + "." + String(PROJECT_VERSION_PATCH));
    delay(10);

    #ifdef TEST_NAME // TEST_NAME is defined in credentials.h, if not defined, the device name is loaded from NV RAM
        String LoadedDeviceName = "TTGO-T-SIM7070G_3";
    #else
        String LoadedDeviceName = loadDeviceName(); // Load device name from NV RAM
        deviceName = (char*)malloc(LoadedDeviceName.length() + 1);
        if (deviceName != nullptr) {
                        // Copy the payload into the newly allocated memory
                        strncpy(deviceName, LoadedDeviceName.c_str(), LoadedDeviceName.length());
                        deviceName[LoadedDeviceName.length()] = '\0'; // Null-terminate the string
                        Serial.print("New deviceName: ");
                        Serial.println(deviceName);
                    } else {
                        // Handle memory allocation error
                        Serial.println("Error: Memory allocation failed for deviceName.");
    
                    }
    #endif TEST_NAME

    pinMode(LED_PIN, OUTPUT);     
    digitalWrite(LED_PIN, HIGH);    // Clear Blue LED
    digitalWrite(LED_PIN, LOW);     // Set LED ON = Insert SD-card !!!

    modemPowerOn();                 // Power on the modem

    digitalWrite(LED_PIN, HIGH);    // Set LED OFF = No longer insert SC-card

    setupSDCard();                // Setup SD-card
    logAndPublish("log", "Wait for modem initialisation" ,LOG_INFO);
    //delay(1000);

    // Set-up modem serial communication
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    Serial.println("Initializing modem...");
    if (!modem.init()) { // Restart takes quite some time. To skip it, call init() instead of restart()
        logAndPublish("log","Failed to restart modem, attempting to continue without restarting", LOG_INFO);
    } else {
        logAndPublish("log","Modem started", LOG_INFO);
    }

    String modemName = modem.getModemName();
    logAndPublish("modem",modemName, LOG_INFO);
    
    #if TINY_GSM_USE_GPRS
    // Unlock your SIM card with a PIN if needed
    if (GSM_PIN && modem.getSimStatus() != 3) {
        modem.simUnlock(GSM_PIN);
    }
    #endif

    #if TINY_GSM_USE_WIFI
    // Wifi connection parameters must be set before waiting for the network
    SerialMon.print(F("Setting SSID/password..."));
    if (!modem.networkConnect(wifiSSID, wifiPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");
    #endif

    SerialMon.print("Waiting for network...");
    if (!modem.waitForNetwork()) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");
    if (modem.isNetworkConnected()) {
        SerialMon.println("Network connected");
    }

    #if TINY_GSM_USE_GPRS
    // GPRS connection parameters are usually set after network registration
    SerialMon.print(F("Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
        SerialMon.println(" fail");
        delay(10000);
        return;
    }
    SerialMon.println(" success");

    if (modem.isGprsConnected()) {
        SerialMon.println("GPRS connected");
    }
    #endif
    
    // Sync ESP local time with network
    syncTimeWithNetwork(modem);

    // MQTT Broker setup
    mqtt.setServer(mqttBroker, mqttPort);
    mqtt.setCallback(mqttCallback);
    mqttConnect();
    subscribeToTopics();
    fullTopic = baseTopic + "/" +(deviceName)+ "/" + topicDeviceName;
    Serial.println("Publishing device name to " + fullTopic);   
    mqtt.publish(fullTopic.c_str(), deviceName, true);
}

//****** LOOP ******//
void loop()
{   
    // Get the current time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    logAndPublish("time", asctime(&timeinfo) ,LOG_INFO);
    logAndPublish("status", "Waking up",LOG_INFO);
   
    // Make sure we're still registered on the network
    if (!modem.isNetworkConnected()) {
        SerialMon.println("Network disconnected");
        if (!modem.waitForNetwork(60000L, true)) {
            SerialMon.println(" fail");
            delay(10000);
            ESP.restart();
            return;
        }
        if (modem.isNetworkConnected()) {
            SerialMon.println("Network re-connected");
        }

#if TINY_GSM_USE_GPRS
        // and make sure GPRS/EPS is still connected
        if (!modem.isGprsConnected()) {
            SerialMon.println("GPRS disconnected!");
            SerialMon.print(F("Connecting to "));
            SerialMon.print(apn);
            if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
                SerialMon.println(" fail");
                delay(10000);
                return;
            }
            if (modem.isGprsConnected()) {
                SerialMon.println("GPRS reconnected");
            }
        }
#endif
    }
    if (!mqtt.connected()) {
        SerialMon.println("=== MQTT NOT CONNECTED ===");
        // Reconnect every 10 seconds
        uint32_t t = millis();
        if (t - lastReconnectAttempt > 10000L) {
            lastReconnectAttempt = t;
            if (mqttConnect()) {
                lastReconnectAttempt = 0;
                subscribeToTopics();
            }
        }
        delay(100);
        return;
    }
    currentBatteryVoltage = readBatteryVoltage();
    logAndPublish("currentBatteryVoltage", String(currentBatteryVoltage),LOG_INFO);
    syncTimeWithNetwork(modem);

    SerialMon.println("=== MQTT IS CONNECTED ===");
    SerialMon.println(deviceName);
    mqtt.loop();
    // If USE_DEEP_SLEEP is defined, the system will go to deep sleep else it will just wait KEEP_ALIVE_TIME seconds    
    #ifdef USE_DEEP_SLEEP
        logAndPublish("status", "Going to deep sleep",LOG_INFO);
        Serial.flush();
        modemPowerOff(); // Power off the modem
        esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*1000000); // Sleep for TIME_TO_SLEEP time in microseconds
        esp_deep_sleep_start();
    #else
        logAndPublish("status", "Going to sleep",LOG_INFO);
        Serial.println("Going to sleep for " + String(KEEP_ALIVE_TIME) + " seconds");
        delay(KEEP_ALIVE_TIME*1000);
    #endif USE_DEEP_SLEEP
}
