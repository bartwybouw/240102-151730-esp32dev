/**************************************************************
*
* Test client developed for demonstration
* HW ESP32 + GSM Module + SD-card reader, battery, GPS
* Lilygo TTGO T-SIM7070G
* 
* Based on example sketch MQTTClient
*
* Subscribe to the topic GsmClientTest/ledStatus
* Publish "toggle" to the topic GsmClientTest/led and the LED on your board
* should toggle and you should see a new message published to
* GsmClientTest/ledStatus with the newest LED status.
* 
* Goal is to 
*  - read GPS location and store (time-stamped) on the SD-card
*  - read battery status and store (time-stamped) on the SD-card
*  - gather GPIO-data and store (time stamped) on the SD-card
*  - when possible  
*    connect to the network 
*        - send a certain amount of the stored data using MQTT (determined how often and how fast the connection is)
*        - receive parameter updates, if any
*        - be able to receive software updates
* Insights
*  - amount of data being sent
*  - how well does this scaling up/down of data work and the impact on cost
* Bart Wybouw 29/12/2023 with the support of ChatGPT 4.0
*
*/