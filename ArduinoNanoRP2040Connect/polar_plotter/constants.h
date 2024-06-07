#ifndef _POLAR_PLOTTER_CONSTANTS_
#define _POLAR_PLOTTER_CONSTANTS_

#define USE_BLE 1
#define USE_CLOUD 0
#define USE_SERIAL 1

#define INPUT_CHECK_WAIT_TIME 200UL

#define BLE_DEVICE_NAME "Dynamic_Sand_Arduino"
#define BLE_SERVICE_UUID "45aa5c8f-c47e-42f6-af4a-66544b8aff17"
#define BLE_STATUS_UUID "52eb19a4-6421-4910-a8ca-7ff75ef2f56b"
#define BLE_COMMAND_UUID "66af95bc-3dd1-4343-b4b5-ad328b33fda7"
#define BLE_MAX_RADIUS_UUID "fb65af7d-494f-4a45-8872-6e6ffbf0703c"
#define BLE_RADIUS_STEP_SIZE_UUID "fd18cf65-85d7-4730-ad77-4cc3fabaab99"
#define BLE_AZIMUTH_STEP_SIZE_UUID "4dc9c338-0004-4c05-bd26-bb13e55c3bb9"
#define BLE_MARBLE_SIZE_UUID "60af168a-b702-4d0b-8c1b-f35c7a436781"
#define BLE_DRAWING_UUID "fa95bee6-46f9-4898-913a-0575019d3d33"
#define BLE_STEP_UUID "54a63a69-90ce-4b14-a103-46152bb1da68"
#define BLE_RADIUS_UUID "7fcd311a-fafa-47ee-80b8-618616697a59"
#define BLE_AZIMUTH_UUID "eb654acc-3430-45e3-8dc9-22c9fe982518"
#define BLE_STATE_UUID "ec314ea1-7426-47fb-825c-8fbd8b02f7fe"
#define BLE_STRING_SIZE 512
#define BLE_DEBUG 0

#define DEFAULT_DEBUG_LEVEL 0
#define DRAWINGS_FILE "/drawings.json"
#define DRAWING_PATH_PREFIX "/drawings/"

#define LCD_RS 6
#define LCD_EN 7
#define LCD_D4 2
#define LCD_D5 3
#define LCD_D6 4
#define LCD_D7 5

#define RADIUS_ADDRESS 0
#define RADIUS_STEP_PIN 14
#define RADIUS_DIR_PIN 15

#define AZIMUTH_ADDRESS 1
#define AZIMUTH_STEP_PIN 16
#define AZIMUTH_DIR_PIN 17

#define MAX_RADIUS 1000
#define MARBLE_SIZE_IN_RADIUS_STEPS 650

const double maxRadius = MAX_RADIUS;
const double marbleSizeInRadiusSteps = MARBLE_SIZE_IN_RADIUS_STEPS;

#define INITIAL_BACKOFF_DELAY_MILLIS 500UL
#define MAX_BACKOFF_DELAY_MILLIS 300000UL

#endif
