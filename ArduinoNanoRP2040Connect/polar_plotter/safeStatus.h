#include <Arduino.h>
#include "statusUpdate.h"
#include "constants.h"

#if USE_LCD > 0
#include <LiquidCrystal.h>
#endif

class SafeStatus : public StatusUpdate {
private:
#if USE_LCD > 0
  LiquidCrystal lcd;
#endif
  String lastKey;
  String lastValue;
  bool preservePair;

  void safeLcdPrint(const String &value);

public:
  SafeStatus();
  void init();

  void writeStatus(const String &key, const String &value);

  void save();
  void restore();
};
