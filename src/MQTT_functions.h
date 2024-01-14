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

enum MqttTopic {
    TOPIC_LED,
    TOPIC_DEVICENAME,  
    TOPIC_REFRESH_TIME,
    TOPIC_UNKNOWN
};

void mqttCallback(char *topic, byte *payload, unsigned int len);
boolean mqttConnect();
void subscribeToTopics();

// Convert topics
MqttTopic getTopicType(const char* topic) {
    Serial.println("getTopicType");
    if (String(topic) == topicLed) {
        return TOPIC_LED;
    }
    if (String(topic) == topicDeviceName) {
        return TOPIC_DEVICENAME;
    }
    if (String(topic) == topicRefreshTime) {
        return TOPIC_REFRESH_TIME;
    }
    return TOPIC_UNKNOWN;
}

const char* getTopicString(MqttTopic topic) {
    switch (topic) {
        case TOPIC_LED:
            return topicLed;
        case TOPIC_DEVICENAME:
            return topicDeviceName;
        case TOPIC_REFRESH_TIME:
            return topicRefreshTime;
        // Add other cases as needed
        default:
            return nullptr;
    }
}

void subscribeToTopics() {
    for (int i = 0; i < mqttTopicsCount; ++i) {
        String fullTopic = String(deviceName) + "/" + mqttTopics[i];
        Serial.println("Subscribing to " + fullTopic);
        mqtt.subscribe(fullTopic.c_str());
    }
}

void unsubscribeFromTopics() {
    for (int i = 0; i < mqttTopicsCount; ++i) {
        String fullTopic = String(deviceName) + "/" + mqttTopics[i];
        Serial.println("Subscribing to " + fullTopic);
        mqtt.unsubscribe(fullTopic.c_str());
    }
}


//MQTT Callback
void mqttCallback(char *topic, byte *payload, unsigned int len) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.println("]: ");
    Serial.print("Payload: ");
    Serial.write(payload, len);
    Serial.println();

    // Convert char* topic to String for manipulation
    String strTopic = String(topic);

    // Remove deviceName from the head of the topic
    String processedTopic = strTopic.substring(strlen(deviceName) + 1); // +1 to remove the slash
    Serial.print("Processed Topic: ");
    Serial.println(processedTopic);

    // Check if the topic matches any in the mqttTopics array
    bool topicHandled = false;
    for (int i = 0; i < mqttTopicsCount; ++i) {
        if (processedTopic.endsWith(mqttTopics[i])) {
            // Handle the topic based on the suffix
            // Topic = led
            if (strcmp(mqttTopics[i], "led") == 0) {
                // *** Handle LED topic
                ledStatus = !ledStatus;
                digitalWrite(LED_PIN, ledStatus);
                logAndPublish("log", "Led status changed to " + String(ledStatus), LOG_INFO);
                mqtt.publish(topicLedStatus, ledStatus ? "1" : "0", true);
                topicHandled = true;
                break;
            } else if (strcmp(mqttTopics[i], topicDeviceName) == 0) {
                // *** Topic = Device Name
                // Free the old memory if it was previously allocated
                free(deviceName);

                // Allocate new memory for the updated name (including null-terminator)
                deviceName = (char*)malloc(len + 1);

                if (deviceName != nullptr) {
                    // Copy the payload into the newly allocated memory
                    strncpy(deviceName, (char*)payload, len);
                    deviceName[len] = '\0'; // Null-terminate the string
                    deviceName[len] = '\0';
                    Serial.print("New deviceName: ");
                    Serial.println(deviceName);
                } else {
                    // Handle memory allocation error
                    Serial.println("Error: Memory allocation failed for deviceName.");
                    break;
                }
                logAndPublish("log", "Device name changed to " + String(deviceName), LOG_INFO);
                logAndPublish("log", "Reconnecting to MQTT-broker " + String(deviceName), LOG_INFO);
                mqtt.disconnect();
                delay(1000);  //
                mqttConnect();       
                logAndPublish("log", "Resubscribing to topics with new device name " + String(deviceName), LOG_INFO);
                unsubscribeFromTopics();
                subscribeToTopics();

                topicHandled = true;
                break;
            } else if (strcmp(mqttTopics[i], topicRefreshTime) == 0) {
                // *** Topic Refresh Time
                refreshTime = atoi((char*)payload);
                if (refreshTime == 0) {
                    refreshTime = DEFAULT_REFRESH_TIME;
                }
                topicHandled = true;
                break;
        }
    }
    }
    if (!topicHandled) {
        Serial.println("Unknown topic: " + processedTopic);
    }  else {
        Serial.println("Handled topic: " + processedTopic);
    }
}


// Function to connect and subscribe to topics
boolean mqttConnect()
{
    Serial.print("Connecting to ");
    Serial.println(mqttBroker);
    // Connect to MQTT Broker
    //boolean status = mqtt.connect("GsmClientTest");

    // Or, if you want to authenticate MQTT:
    boolean status = mqtt.connect(deviceName, mqttUser, mqttPassword);

    if (status == false) {
        Serial.println(" fail");
        return false;
    }
    Serial.println(" success");
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