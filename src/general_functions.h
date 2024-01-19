/**************************************************************
 * Bart Wybouw 14/1/2024
 * 
 * This file contains generic functions
 *
 **************************************************************/

// *** Function definitions ***
// ****************************
void saveDeviceName(const char* deviceName);
String loadDeviceName();
float readBatteryVoltage();
void syncTimeWithNetwork(TinyGsm& modem);
String getNetworkDateTime(TinyGsm& modem);
void initiateRestart();
void performDelayedRestart();

// *** Function implementations ***
// ********************************
// Save deviceName to ESP32 NVS (Non-Volatile Storage)
void saveDeviceName(const char* deviceName) {
    preferences.begin("nvs", false); // "general" is the namespace; false for read/write mode
    preferences.putString("deviceName", deviceName);
    preferences.end();
}

// Load deviceName from ESP32 NVS (Non-Volatile Storage)
String loadDeviceName() {
    preferences.begin("general", false); // "general" is the namespace; true for read-only mode
    if (!preferences.isKey("deviceName")) {
    preferences.putString("deviceName", InitDeviceName);
    Serial.println("Device name not found in NVS, setting to default");
    }
    String name = preferences.getString("deviceName", InitDeviceName); // Provide a default name
    preferences.end();
    Serial.println("Device name: " + name);
    return name;
}
    
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

// Sync network time to ESP32 local clock
void syncTimeWithNetwork(TinyGsm& modem) {
    String dateTime = getNetworkDateTime(modem);
    if (dateTime.length() > 0) {
        struct tm tm;
        // Adjust the format specifier according to the actual dateTime format
        if (strptime(dateTime.c_str(), "%y/%m/%d,%H:%M:%S", &tm)) {
            time_t t = mktime(&tm);
            timeval tv = { t, 0 };
            settimeofday(&tv, NULL);

            // Print the synchronized date and time
            char timeString[64];
            strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", &tm);
            Serial.print("Network Time Synced: ");
            Serial.println(timeString);
        } else {
            Serial.println("Failed to parse network time");
        }
    } else {
        Serial.println("Failed to get network time");
    }
}

String getNetworkDateTime(TinyGsm& modem) {
    modem.sendAT("+CCLK?");
    String response;
    if (modem.waitResponse(10000L, response) == 1) {
        // Processing the response to extract the date-time string
        int index = response.indexOf("\"");
        if (index != -1) {
            String dateTime = response.substring(index + 1, response.lastIndexOf("\""));
            return dateTime; // Return the extracted date-time string
        }
    }
    return "";
}

void initiateRestart() {
    if (restartTime == 0) { // Start the timer if it's not already running
        restartTime = millis();
        Serial.println("Restart initiated. Continuing operations for a few seconds...");
        // Add any immediate pre-restart actions here (e.g., send an MQTT message)
    }
}

void performDelayedRestart() {
    if (restartTime > 0 && millis() - restartTime >= delayBeforeRestart) {
        // Perform any final actions here (e.g., send final MQTT message, write logs)
        Serial.println("Performing restart...");
        ESP.restart();
    }
}
