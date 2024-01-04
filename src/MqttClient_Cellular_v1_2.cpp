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
#define CURRENTFILEANDVERSION MqttClient_Cellular_v1_2
#define TINY_GSM_MODEM_SIM7070 // Define modem type
#define SerialMon Serial // Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialAT Serial1 // Set serial for AT commands (to the module), use Hardware Serial on Mega, Leonardo, Micro
#define TINY_GSM_DEBUG SerialMon // Define the serial console for debug prints, if needed

//#define DUMP_AT_COMMANDS // See all AT commands, if wanted

// Standard libraries to include
#include <Arduino.h> // Needed for PlatformIO C++ compatibility
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <Ticker.h>
#include <SPI.h>
#include <SD.h>
#include <esp_adc_cal.h>

Ticker tick;

#define uS_TO_S_FACTOR 1000000ULL  // Conversion factor for micro seconds to seconds
#define TIME_TO_SLEEP  60          // Time ESP32 will go to sleep (in seconds)
#define LED_PIN 12                 // Blue LED on the ESP32 module, used to indicate GPS lock 

// Some global definitions
int ledStatus = LOW;              // Indicates the status of the blue LED
int vref = 1100;                  // For battery measurement
float current_battery_voltage = 9.99; // For battery measurement
uint32_t lastReconnectAttempt = 0;
String baseTopic = "TTGO-T-SIM7070G_2"; // Needs to be configured during the onboarding
time_t now;
struct tm timeinfo;

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
#include "ReadMe.h" // Contains additional info on working of the sketch
#include "credentials.h" // File with credentials
#include "Modem_functions.h" // All modem related functions and definitions 
#include "GPS_functions.h" // All GPS related functions and definitions
#include "MQTT_functions.h" // All MQTT ...
#include "SDcard_functions.h" // All SDcard, storage and storage management ...
#include "Logging.h" // Functions to do logging, translate debug levels and save and transmit log info

// Battery Calculation
float readBatteryVoltage() {
    uint16_t v = analogRead(ADC_PIN);
    return ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
     #ifdef DEBUG 
        Serial.println("Reading battery voltage"); 
    #endif
    // Battery measurement  
    esp_adc_cal_characteristics_t adc_chars;
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        vref = adc_chars.vref;
    }
}

//****** SETUP ******//
void setup()
{
    // Set console baud rate
    Serial.begin(115200);
    delay(10);

    // Set LED OFF
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    pinMode(PWR_PIN, OUTPUT);
    digitalWrite(PWR_PIN, HIGH);
    // Starting the machine requires at least 1 second of low level, and with a level conversion, the levels are opposite
    delay(1000);
    digitalWrite(PWR_PIN, LOW);

    // Prepare SD-card
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI)) {
        logAndPublish("SDcard","MOUNT FAIL",LOG_INFO);
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        logAndPublish("SDcard", String(cardSize) + "MB" ,LOG_INFO);
    }
    float BatteryVoltage=readBatteryVoltage();
    logAndPublish("currentBatteryVoltage",String(BatteryVoltage).c_str(),LOG_INFO);
    // Waiting for modem initialisation
    logAndPublish("log", "Wait..." ,LOG_INFO);
    delay(1000);

    // Set-up modem serial communication
    SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

    // Restart takes quite some time
    // To skip it, call init() instead of restart()
    Serial.println("Initializing modem...");
    if (!modem.restart()) {
        logAndPublish("log","Failed to restart modem, attempting to continue without restarting", LOG_INFO);
    } else {
        logAndPublish("log","Modem started", LOG_INFO);
    }

    String name = modem.getModemName();
    DBG("Modem Name:", name);
    String modemInfo = modem.getModemInfo();
    DBG("Modem Info:", modemInfo);

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

    // MQTT Broker setup
    mqtt.setServer(mqttBroker, mqttPort);
    mqtt.setCallback(mqttCallback);

    // Setup time   
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    /*
    // Wait for time to be synchronized
    time_t now = time(nullptr);
    while (now < 24 * 3600) {
        delay(100);
        now = time(nullptr);
    }
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    logAndPublish(baseTopic + "/localTime", asctime(&timeinfo)); */
    logAndPublish("CurrentFileVersion", "MqttClient_Cellular_v1_2",LOG_INFO);
}

//****** LOOP ******//
void loop()
{   
    // Get the current time
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
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
            }
        }
        delay(100);
        return;
    }
    float currentBatteryVoltage = readBatteryVoltage();
    logAndPublish("currentBatteryVoltage", String(currentBatteryVoltage),LOG_INFO);
    SerialMon.println("=== MQTT IS CONNECTED ===");
    mqtt.loop();
    logAndPublish("status", "Going to sleep",LOG_INFO);
    delay(5000);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP*1000000); // Sleep for TIME_TO_SLEEP time in microseconds
    esp_deep_sleep_start();
}
