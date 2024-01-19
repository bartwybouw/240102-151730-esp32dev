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
    TOPIC_UPDATE_TIME,
    TOPIC_USE_DEEP_SLEEP,
    TOPIC_ESP_RESTART,
    TOPIC_UNKNOWN
};
// *** Function definitions ***
MqttTopic getTopicType(const char* topic);
const char* getTopicString(MqttTopic topic) ;
void subscribeToTopics();
void unsubscribeFromTopics();
void mqttCallback(char *topic, byte *payload, unsigned int len);
void updateDeviceName(const char* newName, unsigned int len);
boolean mqttConnect();

// *** Function implementations ***
// Convert topics
MqttTopic getTopicType(const char* topic) {
    Serial.println("getTopicType");
    if (String(topic) == topicLed) {
        return TOPIC_LED;
    }
    if (String(topic) == topicDeviceName) {
        return TOPIC_DEVICENAME;
    }
    if (String(topic) == topicUpdateTime) {
        return TOPIC_UPDATE_TIME;
    }
    if (String(topic) == topicUseDeepSleep) {
        return TOPIC_USE_DEEP_SLEEP;
    }
    if (String(topic) == topicEspRestart) {
        return TOPIC_ESP_RESTART;
    }
    
    return TOPIC_UNKNOWN;
}

const char* getTopicString(MqttTopic topic) {
    switch (topic) {
        case TOPIC_LED:
            return topicLed;
        case TOPIC_DEVICENAME:
            return topicDeviceName;
        case TOPIC_UPDATE_TIME:
            return topicUpdateTime;
        case TOPIC_USE_DEEP_SLEEP:
            return topicUseDeepSleep;
        case TOPIC_ESP_RESTART:
            return topicEspRestart;
        // Add other cases as needed
        default:
            return nullptr;
    }
}

void publishParameterTopics() {
    // Publish the device name
    fullTopic = baseTopic + "/" + deviceName + "/" + topicDeviceName;
    mqtt.publish(fullTopic.c_str(), deviceName, true);
    // Publish the update time
    fullTopic = baseTopic + "/" + deviceName + "/" + topicUpdateTime;
    mqtt.publish(fullTopic.c_str(), String(updateTime).c_str(), true);
    // Publish the use deep sleep
    fullTopic = baseTopic + "/" + deviceName + "/" + topicUseDeepSleep;
    mqtt.publish(fullTopic.c_str(), String(useDeepSleep).c_str(), true);
    // Publish the esp restart
    fullTopic = baseTopic + "/" + deviceName + "/" + topicEspRestart;
    mqtt.publish(fullTopic.c_str(), String(espRestart).c_str(), true);
}

void subscribeToTopics() {
    #ifdef SUBSCRIBE_TO_ALL_TOPICS
        String fullTopic = baseTopic + "/#";
        Serial.println("Subscribing to " + fullTopic);
        mqtt.subscribe(fullTopic.c_str());
    #else
        for (int i = 0; i < mqttTopicsCount; ++i) {
            String fullTopic = String(baseTopic) + "/" + String(deviceName) + "/SET/" + mqttTopics[i];
            Serial.println("Subscribing to " + fullTopic);
            mqtt.subscribe(fullTopic.c_str());
        }
    #endif
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
    messageProcessed = true;     // Set the flag
    DEBUG_PRINT("Message arrived [");
    DEBUG_PRINT(topic);
    DEBUG_PRINTLN("]");

    // Convert payload to a null-terminated string
    char message[len + 1];
    memcpy(message, payload, len);
    message[len] = '\0';

    DEBUG_PRINT("Payload: =");
    DEBUG_PRINT(message);
    DEBUG_PRINTLN("=");

    // Process the topic
    String strTopic = String(topic);
    String processedTopic = strTopic.substring(strTopic.lastIndexOf('/') + 1);
    DEBUG_PRINT("Processed Topic: ");
    DEBUG_PRINTLN(processedTopic);

    // Check if the topic matches any in the mqttTopics array
    bool topicHandled = false;
    for (int i = 0; i < mqttTopicsCount; ++i) {
        if (processedTopic.equals(mqttTopics[i])) {
            //  Handle the topic based on the suffix
            if (strcmp(mqttTopics[i], "led") == 0) {
                // *** Handle LED topic
                ledStatus = !ledStatus;
                digitalWrite(LED_PIN, ledStatus);
                logAndPublish("log", "Led status changed to " + String(ledStatus), LOG_INFO);
                fullTopic = baseTopic + "/" + deviceName + "/" + topicLedStatus;
                mqtt.publish(fullTopic.c_str(), ledStatus ? "1" : "0", true);
                topicHandled = true;
                break;
            } else if (strcmp(mqttTopics[i], topicDeviceName) == 0) {
                // *** Handle device name update
                if (deviceName != (char*)payload) {
                    updateDeviceName(message, len);
                    logAndPublish("log", "Device name changed to " + String(deviceName), LOG_INFO);
                    topicHandled = true;
                    break;  /* code */
                } else { // Same name
                    logAndPublish("log", "Device name not changed", LOG_INFO);
                    topicHandled = true;
                    break;
                }
            } else if (strcmp(mqttTopics[i], topicUpdateTime) == 0) {
                // *** Topic Update Time
                updateTime = atoi((char*)payload);
                if (updateTime == 0) {
                    updateTime = DEFAULT_UPDATE_TIME;
                }
                logAndPublish("log", "Update time changed to " + String(updateTime), LOG_INFO);
                topicHandled = true;
                break;
            } else if (strcmp(mqttTopics[i], topicUseDeepSleep) == 0) {
                // *** Topic use Deep Sleep
                String fullPayload;
                if (String(message) == "1") {
                    useDeepSleep = true;
                    fullPayload = "useDeepSleep state changed to true ";
                } else {
                    useDeepSleep = false;
                    fullPayload = "useDeepSleep state changed to false ";
                }

                DEBUG_PRINTLN("Full payload: " + fullPayload);
                logAndPublish("log", fullPayload, LOG_INFO);
                topicHandled = true;
                break;
            }  else if (strcmp(mqttTopics[i], topicEspRestart) == 0) {
                // *** Topic use Deep Sleep
                espRestart = true;
                logAndPublish("log", "Restarting ESP", LOG_INFO);
                
                topicHandled = true;
                break;
            } else {
                Serial.println("Unknown topic: " + processedTopic);
                topicHandled = true;
                break;
            }

            // Add other topic handlers here
        }
    }

    if (!topicHandled) {
        Serial.println("Unknown topic: " + processedTopic);
    } else {
        Serial.println("Handled topic: " + processedTopic);
    }
}

void updateDeviceName(const char* newName, unsigned int len) {
    if (deviceName != nullptr) {
        free(deviceName);
    }
    deviceName = (char*)malloc(len + 1);
    if (deviceName != nullptr) {
        strncpy(deviceName, newName, len);
        deviceName[len] = '\0';
        Serial.print("New deviceName: ");
        Serial.println(deviceName);
    } else {
        Serial.println("Error: Memory allocation failed for deviceName.");
    }
    // Additional logic for handling new device name
}



// Function to connect and subscribe to topics
boolean mqttConnect()
{
    Serial.print("Connecting to ");
    Serial.println(mqttBroker);
    // Connect to MQTT Broker
    //boolean status = mqtt.connect("GsmClientTest");

    // Or, if you want to authenticate MQTT:
    logAndPublish("log", "deviceName : " + String(deviceName) , LOG_INFO);  
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