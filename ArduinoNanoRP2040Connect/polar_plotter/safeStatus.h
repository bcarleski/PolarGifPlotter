#include <Arduino.h>
#include "statusUpdate.h"
#include "constants.h"
#include "safePrinter.h"

#if USE_LCD > 0
#include <LiquidCrystal.h>
#endif
#if USE_BLE > 0
#include <ArduinoBLE.h>
#endif

#define POSITION_UPDATE_INTERVAL 500
#define STEP_UPDATE_INTERVAL 500

class SafeStatus : public StatusUpdate {
private:
#if USE_LCD > 0
  LiquidCrystal lcd;
#endif
#if USE_BLE > 0
  BLEService& bleService;
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
#endif
  SafePrinter printer;
  String lastKey;
  String lastValue;
  bool preservePair;
  unsigned long nextPositionUpdate;
  unsigned long nextStepUpdate;

  void safeLcdPrint(const String &value);

public:
  SafeStatus(SafePrinter printer
#if USE_BLE > 0
    , BLEService& bleService
#endif
  );
  void init();

  void writeStatus(const String &key, const String &value);
  void setMaxRadius(const double value);
  void setRadiusStepSize(const double value);
  void setAzimuthStepSize(const double value);
  void setMarbleSizeInRadiusSteps(const int value);
  void setCurrentDrawing(const String &value);
  void setCurrentCommand(const String &value);
  void setCurrentStep(const int value);
  void setPosition(const double radius, const double azimuth);
  void setState(const String &value);

  void save();
  void restore();
};
