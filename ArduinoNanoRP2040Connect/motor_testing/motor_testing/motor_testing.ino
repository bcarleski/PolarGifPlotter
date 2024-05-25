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
#define BLE_POLL_INTERVAL 100
#define POSITION_UPDATE_INTERVAL 500
#define STEP_UPDATE_INTERVAL 500
#define RADIUS_STEPPER_STEP_PIN 15
#define RADIUS_STEPPER_DIR_PIN 16
#define AZIMUTH_STEPPER_STEP_PIN 17
#define AZIMUTH_STEPPER_DIR_PIN 18
#define USE_SERIAL 0

#include <ArduinoBLE.h>
#include <AccelStepper.h>

unsigned long currentStepTime = 0;
unsigned long nextStepUpdateTime = 0;
unsigned long nextPositionUpdateTime = 0;
unsigned long nextBlePollTime = 0;
bool adjustingAzimuth = false;

BLEService bleService(BLE_SERVICE_UUID);
BLEStringCharacteristic bleCommand(BLE_COMMAND_UUID, BLEWrite | BLERead, BLE_STRING_SIZE);
BLEDoubleCharacteristic bleMaxRadius(BLE_MAX_RADIUS_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleRadiusStepSize(BLE_RADIUS_STEP_SIZE_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleAzimuthStepSize(BLE_AZIMUTH_STEP_SIZE_UUID, BLERead | BLENotify);
BLEIntCharacteristic bleMarbleSize(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify);
BLEStringCharacteristic bleStatus(BLE_STATUS_UUID, BLERead | BLENotify, BLE_STRING_SIZE);
BLEStringCharacteristic bleDrawing(BLE_DRAWING_UUID, BLERead | BLENotify, BLE_STRING_SIZE);
BLEStringCharacteristic bleDrawingCommand(BLE_STEP_UUID, BLERead | BLENotify, BLE_STRING_SIZE);
BLEIntCharacteristic bleStep(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleRadius(BLE_RADIUS_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleAzimuth(BLE_AZIMUTH_UUID, BLERead | BLENotify);
BLEStringCharacteristic bleState(BLE_STATE_UUID, BLERead | BLENotify, BLE_STRING_SIZE);

AccelStepper radiusStepper(AccelStepper::DRIVER, RADIUS_STEPPER_STEP_PIN, RADIUS_STEPPER_DIR_PIN);
AccelStepper azimuthStepper(AccelStepper::DRIVER, AZIMUTH_STEPPER_STEP_PIN, AZIMUTH_STEPPER_DIR_PIN);


void setup() {
#if USE_SERIAL > 0
  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  if (Serial) Serial.println("Starting");
#endif
  delay(2000);
  bleInitialize(true);

  // add the characteristic to the service
  bleService.addCharacteristic(bleMaxRadius);
  bleService.addCharacteristic(bleRadiusStepSize);
  bleService.addCharacteristic(bleAzimuthStepSize);
  bleService.addCharacteristic(bleMarbleSize);
  bleService.addCharacteristic(bleStatus);
  bleService.addCharacteristic(bleDrawing);
  bleService.addCharacteristic(bleDrawingCommand);
  bleService.addCharacteristic(bleStep);
  bleService.addCharacteristic(bleRadius);
  bleService.addCharacteristic(bleAzimuth);
  bleService.addCharacteristic(bleState);

  setMaxRadius(10500);
  setMarbleSizeInRadiusSteps(650);
  setRadiusStepSize(0.0);
  setAzimuthStepSize(0.0);
  setCurrentDrawing("-");
  setCurrentCommand("-");
  setCurrentStep(0);
  setPosition(0, 0);
  setState("Manual");
  bleService.addCharacteristic(bleCommand);
  BLE.addService(bleService);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  bleCommand.setEventHandler(BLEWritten, bleCommandWritten);
  bleCommand.setValue("");

  bleAdvertise();

  radiusStepper.setMaxSpeed(300);
  radiusStepper.setAcceleration(100);

  azimuthStepper.setMaxSpeed(300);
  azimuthStepper.setAcceleration(100);

  delay(2000);

#if USE_SERIAL > 0
  if (Serial) Serial.println("Finished Setup");
#endif
  writeStatus("FINISHED SETUP", "");
}

void loop() {
  if (radiusStepper.distanceToGo() == 0 && radiusStepper.currentPosition() != 0) radiusStepper.moveTo(-radiusStepper.currentPosition());
  if (azimuthStepper.distanceToGo() == 0 && azimuthStepper.currentPosition() != 0) azimuthStepper.moveTo(-azimuthStepper.currentPosition());

  radiusStepper.run();
  azimuthStepper.run();

  currentStepTime = millis();

  if (nextBlePollTime <= currentStepTime)  {
    BLE.poll();
    nextBlePollTime = currentStepTime + BLE_POLL_INTERVAL;
  }
}
void bleInitialize(bool holdOnFailure) {
#if USE_SERIAL > 0
  if (Serial) Serial.println("Starting BLE");
#endif

  int bleBegin = BLE.begin();
  if (!bleBegin) {
#if USE_SERIAL > 0
    if (Serial) {
      Serial.print(" BLE Start failed - ");
      Serial.println(bleBegin);
    }
#endif

    while (holdOnFailure);
  } else {
    // set advertised local name and service UUID:
    BLE.setLocalName(BLE_DEVICE_NAME);
    BLE.setAdvertisedService(bleService);
  }
}

void bleCommandWritten(BLEDevice central, BLECharacteristic characteristic) {
  String command = bleCommand.value();
#if USE_SERIAL > 0
  if (Serial) Serial.println("Got new command from BLE: " + command);
#endif

  const char chr = command.charAt(0);
  switch (chr)
  {
    case 'A': // Manual Azimuth
    case 'a':
#if USE_SERIAL > 0
      if (Serial) Serial.println("Switching to azimuth");
#endif
      adjustingAzimuth = true;
      radiusStepper.moveTo(0);
      azimuthStepper.moveTo(20000);
      break;
    case 'R': // Manual Radius
    case 'r':
#if USE_SERIAL > 0
      if (Serial) Serial.println("Switching to radius");
#endif
      adjustingAzimuth = false;
      radiusStepper.moveTo(20000);
      azimuthStepper.moveTo(0);
      break;
    case 'X': // Manual Radius
    case 'x':
#if USE_SERIAL > 0
      if (Serial) Serial.println("Stopping");
#endif
      radiusStepper.moveTo(0);
      azimuthStepper.moveTo(0);
      break;
    case 'O': // Offset
    case 'o':
      int accel = command.substring(1).toInt();
      int speed = accel * 4;
#if USE_SERIAL > 0
      if (Serial) Serial.print("Adjusting to ");
      if (Serial) Serial.println(speed);
#endif
      if (!adjustingAzimuth) {
        radiusStepper.setMaxSpeed(speed);
        radiusStepper.setAcceleration(accel);
      } else {
        azimuthStepper.setMaxSpeed(speed);
        azimuthStepper.setAcceleration(accel);
      }
      break;
  }
}

void blePeripheralConnectHandler(BLEDevice central) {
#if USE_SERIAL > 0
  if (!Serial) return;

  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
#endif
}

void blePeripheralDisconnectHandler(BLEDevice central) {
#if USE_SERIAL > 0
  if (!Serial) return;

  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
#endif
}

void bleAdvertise() {
  // start advertising
  BLE.advertise();
#if USE_SERIAL > 0
  if (Serial) Serial.println("BLE waiting for connections");
#endif
}

void setStringValue(BLECharacteristic &characteristic, const char * name, const String &value) {
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

  int ret = characteristic.writeValue(val.c_str(), false);
}

void setDoubleValue(BLECharacteristic &characteristic, const char * name, const double value) {
  byte arr[8];
  memcpy(arr, (uint8_t *) &value, 8);
  int ret = characteristic.writeValue(arr, 8, false);
}

void setIntValue(BLECharacteristic &characteristic, const char * name, const int value) {
  byte arr[4];
  memcpy(arr, (uint8_t *) &value, 4);
  int ret = characteristic.writeValue(arr, 4, false);
}

void setMaxRadius(const double value) {
  setDoubleValue(bleMaxRadius, "Max Radius", value);
}

void setRadiusStepSize(const double value) {
  setDoubleValue(bleRadiusStepSize, "Radius Step Size", value);
}

void setAzimuthStepSize(const double value) {
  setDoubleValue(bleAzimuthStepSize, "Azimuth Step Size", value);
}

void setMarbleSizeInRadiusSteps(const int value) {
  setIntValue(bleMarbleSize, "Marble Size in Radius Steps", value);
}

void setCurrentDrawing(const String &value) {
  setStringValue(bleDrawing, "Drawing", value);
}

void setCurrentCommand(const String &value) {
  setStringValue(bleDrawingCommand, "Command", value);
}

void setCurrentStep(const int value) {
  if (currentStepTime < nextStepUpdateTime) return;

  nextStepUpdateTime = currentStepTime + STEP_UPDATE_INTERVAL;
  setIntValue(bleStep, "Step", value);
}

void setPosition(const double radius, const double azimuth) {
  unsigned long cur = millis();
  if (currentStepTime < nextPositionUpdateTime) return;

  nextPositionUpdateTime = currentStepTime + POSITION_UPDATE_INTERVAL;
  setDoubleValue(bleRadius, "Radius", radius);
  setDoubleValue(bleAzimuth, "Azimuth", azimuth);
}

void setState(const String &value) {
  setStringValue(bleState, "State", value);
}

void writeStatus(const String &key, const String &value) {
  setStringValue(bleStatus, "Status", key + (key.length() > 0 && value.length() > 0 ? ": " : "") + value);
}
