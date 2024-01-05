/**************************************************************
 * Bart Wybouw 31/12/2023
 * 
 * This file contains the MQTT related functions and definitions
 *  - 
 *  - 
 *  - 
 *
 **************************************************************/
#ifndef MQTT_FUNCTIONS_H
#define MQTT_FUNCTIONS_H

void mqttCallback(char *topic, byte *payload, unsigned int len);
boolean mqttConnect();

//extern int ledStatus;

enum MqttTopic {
    TOPIC_LED,
    TOPIC_DEVICENAME,  
    TOPIC_UNKNOWN
};

// Convert topics
MqttTopic getTopicType(const char* topic) {
    if (String(topic) == topicLed) {
        return TOPIC_LED;
    }
    if (String(topic) == topicDeviceName) {
        return TOPIC_DEVICENAME;
    }
    return TOPIC_UNKNOWN;
}

//MQTT
void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.write(payload, len);
    Serial.println();

    // Only proceed if incoming message's topic matches
    MqttTopic topicType = getTopicType(topic);
    switch (topicType) {
        case TOPIC_LED : {
            ledStatus = !ledStatus;
            digitalWrite(LED_PIN, ledStatus);
            logAndPublish("log", "Led status changed to " + String(ledStatus), LOG_INFO);
            mqtt.publish(topicLedStatus, ledStatus ? "1" : "0", true);
            break;
            }   
        case TOPIC_DEVICENAME :{
            // Free the old memory if it was previously allocated
            free(mqttClientName);

            // Allocate new memory for the updated name (including null-terminator)
            mqttClientName = (char*)malloc(len + 1);

            if (mqttClientName != nullptr) {
                // Copy the payload into the newly allocated memory
                strncpy(mqttClientName, (char*)payload, len);
                mqttClientName[len] = '\0'; // Null-terminate the string
            } else {
                // Handle memory allocation error
                Serial.println("Error: Memory allocation failed for mqttClientName.");
                break;
            }

            logAndPublish("log", "Device name changed to " + String(mqttClientName), LOG_INFO);
            break;
            }
        // Add other cases as needed
        case TOPIC_UNKNOWN : {
            Serial.println("Unknown topic");
            break;
            }
    }
}

boolean mqttConnect()
{
    Serial.print("Connecting to ");
    Serial.print(mqttBroker);
    // Connect to MQTT Broker
    //boolean status = mqtt.connect("GsmClientTest");

    // Or, if you want to authenticate MQTT:
     boolean status = mqtt.connect(mqttClientName, mqttUser, mqttPassword);

    if (status == false) {
        Serial.println(" fail");
        return false;
    }
    Serial.println(" success");
    mqtt.publish(baseTopic.c_str(), "GsmClientTest started", true);
    mqtt.subscribe(topicLed);
    return mqtt.connected();
}
/*
// Function to send data over MQTT
void sendDataOverMQTT(const String& data) {
    // Assuming mqttClient is a PubSubClient object
    mqtt.publish("your/topic", data.c_str());
}

// Function to check if data should be sent based on type
bool shouldSendData(DataType type) {
    // Define your logic for filtering data types
    return type == DATA_POINT || type == WARNING_MESSAGE;
}

// Function to process and send data from SD card
void processAndSendDataFromSD() {
    dataFile = SD.open("datalog.txt");
    if (dataFile) {
        while (dataFile.available()) {
            String line = dataFile.readStringUntil('\n');
            int delimiterPos = line.indexOf(',');
            DataType type = static_cast<DataType>(line.substring(0, delimiterPos).toInt());
            String data = line.substring(delimiterPos + 1);

            if (shouldSendData(type)) {
                sendDataOverMQTT(data);
                // You can mark this data as sent in your file or use another method to keep track
            }
        }
        dataFile.close();
    } else {
        // Handle error
    }
}
*/
#endif // MQTT_FUNCTIONS_H