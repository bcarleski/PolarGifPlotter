//#include <BTstackLib.h>
#include <ArduinoBLE.h>

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

BLEService bleService(BLE_SERVICE_UUID);
BLEStringCharacteristic bleCommand(BLE_COMMAND_UUID, BLEWrite | BLERead, BLE_STRING_SIZE);
BLEDoubleCharacteristic bleMaxRadius(BLE_MAX_RADIUS_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleRadiusStepSize(BLE_RADIUS_STEP_SIZE_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleAzimuthStepSize(BLE_AZIMUTH_STEP_SIZE_UUID, BLERead | BLENotify);
BLEIntCharacteristic bleMarbleSize(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify);
BLEStringCharacteristic bleStatus(BLE_STATUS_UUID, BLERead | BLENotify, BLE_STRING_SIZE);
BLEStringCharacteristic bleDrawing(BLE_DRAWING_UUID, BLERead | BLENotify, BLE_STRING_SIZE);
BLEStringCharacteristic bleCurrentCommand(BLE_STEP_UUID, BLERead | BLENotify, BLE_STRING_SIZE);
BLEIntCharacteristic bleStep(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleRadius(BLE_RADIUS_UUID, BLERead | BLENotify);
BLEDoubleCharacteristic bleAzimuth(BLE_AZIMUTH_UUID, BLERead | BLENotify);
BLEStringCharacteristic bleState(BLE_STATE_UUID, BLERead | BLENotify, BLE_STRING_SIZE);

void bleCommandWritten(BLEDevice central, BLECharacteristic characteristic) {
  String command = bleCommand.value();
  Serial.print("BLE input: ");
  Serial.println(command);
}

void blePeripheralConnectHandler(BLEDevice central) {
  String address = central.address();
  Serial.print("Connected event, central: ");
  Serial.println(address);
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  String address = central.address();
  Serial.print("Disconnected event, central: ");
  Serial.println(address);
}

void setStringValue(BLECharacteristic &characteristic, const String &value) {
  String val = value;
  if (val.length() > BLE_STRING_SIZE) {
    val = val.substring(0, BLE_STRING_SIZE);
  }

  characteristic.writeValue(val.c_str(), false);
}

void setDoubleValue(BLECharacteristic &characteristic, const double value) {
  byte arr[8];
  memcpy(arr, (uint8_t *) &value, 8);
  characteristic.writeValue(arr, 8, false);
}

void setIntValue(BLECharacteristic &characteristic, const int value) {
  byte arr[4];
  memcpy(arr, (uint8_t *) &value, 4);
  characteristic.writeValue(arr, 4, false);
}


void setupBluetooth() {
  int bleBegin = BLE.begin();
  if (!bleBegin) {
    Serial.print(" BLE Start failed - ");
    Serial.println(bleBegin);

    while (true);
  } else {
    // set advertised local name and service UUID:
    BLE.setLocalName(BLE_DEVICE_NAME);
    BLE.setAdvertisedService(bleService);
  }

  bleService.addCharacteristic(bleMaxRadius);
  bleService.addCharacteristic(bleRadiusStepSize);
  bleService.addCharacteristic(bleAzimuthStepSize);
  bleService.addCharacteristic(bleMarbleSize);
  bleService.addCharacteristic(bleStatus);
  bleService.addCharacteristic(bleDrawing);
  bleService.addCharacteristic(bleCurrentCommand);
  bleService.addCharacteristic(bleStep);
  bleService.addCharacteristic(bleRadius);
  bleService.addCharacteristic(bleAzimuth);
  bleService.addCharacteristic(bleState);
  bleService.addCharacteristic(bleCommand);
  bleCommand.setEventHandler(BLEWritten, bleCommandWritten);
  bleCommand.setValue("");

  setDoubleValue(bleMaxRadius, 1000);
  setIntValue(bleMarbleSize, 650);
  setDoubleValue(bleRadiusStepSize, 0.0);
  setDoubleValue(bleAzimuthStepSize, 0.0);
  setStringValue(bleDrawing, "-");
  setStringValue(bleCurrentCommand, "-");
  setIntValue(bleStep, 0);
  setDoubleValue(bleRadius, 0.0);
  setDoubleValue(bleAzimuth, 0.0);
  setStringValue(bleState, "Testing");
  setStringValue(bleStatus, "Just started");

  BLE.addService(bleService);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
}

void startBluetooth() {
  Serial.println("Calling Bluetooth Setup");
  BLE.advertise();
  Serial.println("BLE waiting for connections");
}

void bluetoothLoop() {
  BLE.poll();
}