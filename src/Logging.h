/**************************************************************
 * Bart Wybouw 2/1/2024
 * 
 * Functions used for logging
 *
 **************************************************************/
enum logLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_NOTICE,
    LOG_WARNING,
    LOG_ERROR,
    LOG_CRITICAL,
    LOG_ALERT,
    LOG_EMERGENCY
};

void logAndPublish(String topic, String payload, enum logLevel);
String logLevelToString(logLevel level);


// Log function
void logAndPublish(String topic, String payload, logLevel level) {
    // Get current time
    time_t now;
    time(&now);
    struct tm* timeinfo = localtime(&now);
    char timeString[64];
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);
  
    // logLevelToString(level); // Convert log level to string, to decided what to do with it
    // For now nothing is done with the log level, but you could use it to decide what to do with the log message
    SerialMon.println(String(topic + ": " + payload));  // Print to serial monitor
    writeToSDCard(asctime(&timeinfo) + topic + ": " + payload); // Write to SD card 
    fullTopic = baseTopic + "/" + deviceName + "/log/" + mqttTopic.c_str();
    mqtt.publish(fullTopic.c_str(), payload.c_str(), true); // Publish to MQTT
}    

String logLevelToString(logLevel level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}