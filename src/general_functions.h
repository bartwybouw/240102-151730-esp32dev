/**************************************************************
 * Bart Wybouw 14/1/2024
 * 
 * This file contains generic functions
 *
 **************************************************************/

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
void syncTimeWithNetwork() {
  String dateTime = modem.getGSMDateTime(DATE_TIME);
  struct tm tm;
  if (strptime(dateTime.c_str(), "%Y/%m/%d,%H:%M:%S+00", &tm)) {
    time_t t = mktime(&tm);
    timeval tv = { t, 0 };
    settimeofday(&tv, NULL);
  }
}