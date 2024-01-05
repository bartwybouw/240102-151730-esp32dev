/* Project versioning
* 
* MAJOR: Incremented for incompatible API changes.
* MINOR: Incremented for adding functionality in a backward-compatible manner.
* PATCH: Incremented for backward-compatible bug fixes.
*/


#define PROJECT_VERSION_MAJOR 1
#define PROJECT_VERSION_MINOR 3
#define PROJECT_VERSION_PATCH 0

/* 1.3

- Removed version from main filename
- cleanup startup code for modem
- keep system up for a bit longer and go through a couple of MQTT send loops
-