/* Project versioning
* 
* MAJOR: Incremented for incompatible API changes.
* MINOR: Incremented for adding functionality in a backward-compatible manner.
* PATCH: Incremented for backward-compatible bug fixes.
*/


#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 4
#define PROJECT_VERSION_PATCH 0

/* ***** 1.4
- Get and store device name on NV RAM of ESP32
- Combined SC-card and log functions into one file
*/

/* ***** 1.3
+ Removed version from main filename
+ Start with an initial MQTTDeviceName and be able to update it once the device is online using MQTT
+ cleanup startup code for modem
+ keep system up for a bit longer and go through a couple of MQTT send loops
+ improved logging
+ improved MQTT callback handling using array
*/