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

//MQTT
void mqttCallback(char *topic, byte *payload, unsigned int len)
{
    SerialMon.print("Message arrived [");
    SerialMon.print(topic);
    SerialMon.print("]: ");
    SerialMon.write(payload, len);
    SerialMon.println();

    // Only proceed if incoming message's topic matches
    if (String(topic) == topicLed) {
        ledStatus = !ledStatus;
        digitalWrite(LED_PIN, ledStatus);
        SerialMon.print("ledStatus:");
        SerialMon.println(ledStatus);
        mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
    }
}

boolean mqttConnect()
{
    SerialMon.print("Connecting to ");
    SerialMon.print(mqttBroker);
    // Connect to MQTT Broker
    //boolean status = mqtt.connect("GsmClientTest");

    // Or, if you want to authenticate MQTT:
     boolean status = mqtt.connect(mqttClientName, mqttUser, mqttPassword);

    if (status == false) {
        SerialMon.println(" fail");
        return false;
    }
    SerialMon.println(" success");
    mqtt.publish(topicInit, "GsmClientTest started");
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