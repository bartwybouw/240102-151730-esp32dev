/**************************************************************
 * Bart Wybouw 1/1/2024
 * 
 * This file contains the SDcard, storage and storage management
 * and logging related functions and definitions
 *  - 
 *  - 
 *  - 
 *
 **************************************************************/
#ifndef SDCARD_LOG_FUNCTIONS_H
#define SDCARD_LOG_FUNCTIONS_H

enum DataType {
    DATA_POINT,
    LOG_MESSAGE,
    WARNING_MESSAGE
};

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

void writeToSDCard(String data);
void archiveDataFile();
void logToSDCard(const String& data, DataType type);
void logAndPublish(String topic, String payload, enum logLevel);
String logLevelToString(logLevel level);

// Define pins for SD-card
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13

File dataFile; // File object for SD card operations
const char* dataFileName = "/log.txt";
const unsigned long maxFileSize = 1024 * 10; // Maximum file size in bytes (e.g., 10 KB)

void setupSDCard() {
    // Prepare SD-card
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(SD_CS, SPI)) {
        logAndPublish("SDcard","MOUNT FAIL",LOG_INFO);
    } else {
        uint32_t cardSize = SD.cardSize() / (1024 * 1024);
        logAndPublish("SDcard", String(cardSize) + "MB" ,LOG_INFO);
    }
}


// Write to SD-card 
void writeToSDCard(String data) {
    //if (digitalRead(LOG_SWITCH_PIN) == LOW) {
        File file = SD.open("/log.txt", FILE_APPEND);
        if (file) {
            //Serial.println("Writing log.txt : " + data );
            file.println(data);
            file.close();
        } else {
            Serial.println("Error opening log.txt");
        }
    //} else {
    //   Serial.println("Error LOG_SWITCH_PIN NOT LOW");
      
    //}
}

// Function to archive the current data file
void archiveDataFile() {
    unsigned long archiveNumber = 0;
    String archiveName;
    do {
        archiveName = "archive" + String(archiveNumber) + ".txt";
        archiveNumber++;
    } while (SD.exists(archiveName.c_str()));
    SD.rename(dataFileName, archiveName.c_str());
    logAndPublish("SDcard","Archived data file as " + archiveName,LOG_INFO);
}

// Function to log data to SD card
void logToSDCard(const String& data, DataType type) {
    dataFile = SD.open(dataFileName, FILE_APPEND);
    if (dataFile) {
        if (dataFile.size() > maxFileSize) {
            dataFile.close();
            archiveDataFile();
            dataFile = SD.open(dataFileName, FILE_APPEND);
        }
        dataFile.println(String(type) + "," + data); // Example format: "0,Some data point"
        dataFile.close();
    } else {
        // Handle error
    }
}

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
    DEBUG_PRINTLN(">>> " +String(topic + ": " + payload));  // Print to serial monitor
    writeToSDCard(timeString + topic + ": " + payload); // Write to SD card 
    fullTopic = baseTopic + "/" + deviceName + "/log/" + topic; // Create full topic
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

#endif // SDCARD_LOG_FUNCTIONS_H
