/**************************************************************
 * Bart Wybouw 2/1/2024
 * 
 * This file contains the logging functions
 *  - 
 *  - 
 *  - 
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
    time(&now);
    localtime_r(&now, &timeinfo);
    SerialMon.println(String(topic + ": " + payload));
    writeToSDCard(asctime(&timeinfo) + topic + ": " + payload);
    String mqttTopic = baseTopic + "/" + topic;
    mqtt.publish(mqttTopic.c_str(), payload.c_str(), true);
    
    logLevelToString(level);
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