#ifndef _POLAR_PLOTTER_SAFESTATUS_
#define _POLAR_PLOTTER_SAFESTATUS_

#include <Arduino.h>
#include "statusUpdate.h"
#include "constants.h"
#include "safePrinter.h"

#if USE_BLE > 0
#include <ArduinoBLE.h>
#endif

#define POSITION_UPDATE_INTERVAL 500
#define STEP_UPDATE_INTERVAL 500

class ExtendedStatusUpdate : public StatusUpdate {
public:
  virtual void setMaxRadius(const double value) = 0;
  virtual void setMarbleSizeInRadiusSteps(const int value) = 0;
};

class SafeStatus : public ExtendedStatusUpdate {
private:
  ExtendedStatusUpdate* bleUpdater;
  SafePrinter& printer;
  String lastKey;
  String lastValue;
  bool preservePair;
  unsigned long nextPositionUpdate;
  unsigned long nextStepUpdate;

public:
  SafeStatus(SafePrinter& printer) : printer(printer) { lastKey = ""; lastValue = ""; }

#if USE_BLE > 0
  void initBle(ExtendedStatusUpdate* updater) { bleUpdater = updater; }
#endif

  void writeStatus(const String &key, const String &value);
  void setMaxRadius(const double value) { if (bleUpdater) bleUpdater->setMaxRadius(value); }
  void setRadiusStepSize(const double value) { if (bleUpdater) bleUpdater->setRadiusStepSize(value); }
  void setAzimuthStepSize(const double value) { if (bleUpdater) bleUpdater->setAzimuthStepSize(value); }
  void setMarbleSizeInRadiusSteps(const int value) { if (bleUpdater) bleUpdater->setMarbleSizeInRadiusSteps(value); }
  void setCurrentDrawing(const String &value) { if (bleUpdater) bleUpdater->setCurrentDrawing(value); }
  void setCurrentCommand(const String &value) { if (bleUpdater) bleUpdater->setCurrentCommand(value); }
  void setCurrentStep(const int value) { if (bleUpdater) bleUpdater->setCurrentStep(value); }
  void setPosition(const double radius, const double azimuth) { if (bleUpdater) bleUpdater->setPosition(radius, azimuth); }
  void setState(const String &value) { if (bleUpdater) bleUpdater->setState(value); }

  void save() { preservePair = true; }
  void restore() { writeStatus(lastKey, lastValue); preservePair = false; }
};

#if USE_BLE > 0
class BleSafeStatus : public ExtendedStatusUpdate {
private:
  BLEDoubleCharacteristic bleMaxRadius;
  BLEDoubleCharacteristic bleRadiusStepSize;
  BLEDoubleCharacteristic bleAzimuthStepSize;
  BLEIntCharacteristic bleMarbleSize;
  BLEStringCharacteristic bleStatus;
  BLEStringCharacteristic bleDrawing;
  BLEStringCharacteristic bleCommand;
  BLEIntCharacteristic bleStep;
  BLEDoubleCharacteristic bleRadius;
  BLEDoubleCharacteristic bleAzimuth;
  BLEStringCharacteristic bleState;
  unsigned long nextPositionMillis;
  unsigned long nextStepMillis;

protected:
  void setStringValue(BLECharacteristic &characteristic, const char * name, const String &value);
  void setDoubleValue(BLECharacteristic &characteristic, const char * name, const double value);
  void setIntValue(BLECharacteristic &characteristic, const char * name, const int value);

public:
  BleSafeStatus()
    : bleMaxRadius(BLEDoubleCharacteristic(BLE_MAX_RADIUS_UUID, BLERead | BLENotify)),
      bleRadiusStepSize(BLEDoubleCharacteristic(BLE_RADIUS_STEP_SIZE_UUID, BLERead | BLENotify)),
      bleAzimuthStepSize(BLEDoubleCharacteristic(BLE_AZIMUTH_STEP_SIZE_UUID, BLERead | BLENotify)),
      bleMarbleSize(BLEIntCharacteristic(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify)),
      bleStatus(BLEStringCharacteristic(BLE_STATUS_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
      bleDrawing(BLEStringCharacteristic(BLE_DRAWING_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
      bleCommand(BLEStringCharacteristic(BLE_STEP_UUID, BLERead | BLENotify, BLE_STRING_SIZE)),
      bleStep(BLEIntCharacteristic(BLE_MARBLE_SIZE_UUID, BLERead | BLENotify)),
      bleRadius(BLEDoubleCharacteristic(BLE_RADIUS_UUID, BLERead | BLENotify)),
      bleAzimuth(BLEDoubleCharacteristic(BLE_AZIMUTH_UUID, BLERead | BLENotify)),
      bleState(BLEStringCharacteristic(BLE_STATE_UUID, BLERead | BLENotify, BLE_STRING_SIZE)) { }
  void init(BLEService& bleService) {
    bleService.addCharacteristic(bleMaxRadius);
    bleService.addCharacteristic(bleRadiusStepSize);
    bleService.addCharacteristic(bleAzimuthStepSize);
    bleService.addCharacteristic(bleMarbleSize);
    bleService.addCharacteristic(bleStatus);
    bleService.addCharacteristic(bleDrawing);
    bleService.addCharacteristic(bleCommand);
    bleService.addCharacteristic(bleStep);
    bleService.addCharacteristic(bleRadius);
    bleService.addCharacteristic(bleAzimuth);
    bleService.addCharacteristic(bleState);
  }

  void writeStatus(const String &key, const String &value) { setStringValue(bleStatus, "Status", key + (key.length() > 0 && value.length() > 0 ? ": " : "") + value); }
  void setMaxRadius(const double value) { setDoubleValue(bleMaxRadius, "Max Radius", value); }
  void setRadiusStepSize(const double value) { setDoubleValue(bleRadiusStepSize, "Radius Step Size", value); }
  void setAzimuthStepSize(const double value) { setDoubleValue(bleAzimuthStepSize, "Azimuth Step Size", value); }
  void setMarbleSizeInRadiusSteps(const int value) { setIntValue(bleMarbleSize, "Marble Size in Radius Steps", value); }
  void setCurrentDrawing(const String &value) { setStringValue(bleDrawing, "Current Drawing", value); }
  void setCurrentCommand(const String &value) { setStringValue(bleCommand, "Current Command", value); }
  void setCurrentStep(const int value) { if (nextStepMillis > millis()) { nextStepMillis = millis() + STEP_UPDATE_INTERVAL; setIntValue(bleStep, "Current Step", value); } }
  void setPosition(const double radius, const double azimuth) { if (nextPositionMillis > millis()) { nextPositionMillis = millis() + POSITION_UPDATE_INTERVAL; setDoubleValue(bleRadius, "Radius", radius); setDoubleValue(bleAzimuth, "Azimuth", azimuth); } }
  void setState(const String &value) { setStringValue(bleState, "State", value); }
};
#endif
#endif
