/**************************************************************
 * Bart Wybouw 1/1/2024
 * 
 * This file contains the SDcard, storage and storage managemnt
 * related functions and definitions
 *  - 
 *  - 
 *  - 
 *
 **************************************************************/
#ifndef SDCARD_FUNCTIONS_H
#define SDCARD_FUNCTIONS_H

enum DataType {
    DATA_POINT,
    LOG_MESSAGE,
    WARNING_MESSAGE
};

void writeToSDCard(String data);
void archiveDataFile();
void logToSDCard(const String& data, DataType type);

// Define pins for SD-card
#define SD_MISO     2
#define SD_MOSI     15
#define SD_SCLK     14
#define SD_CS       13

File dataFile; // File object for SD card operations
const char* dataFileName = "/log.txt";
const unsigned long maxFileSize = 1024 * 10; // Maximum file size in bytes (e.g., 10 KB)

// Write to SD-card 
void writeToSDCard(String data) {
    //if (digitalRead(LOG_SWITCH_PIN) == LOW) {
        File file = SD.open("/log.txt", FILE_APPEND);
        if (file) {
            Serial.println("Writing log.txt : " + data );
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

#endif // SDCARD_FUNCTIONS_H
