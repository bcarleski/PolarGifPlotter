#include <TMCStepper.h>

#define SERIAL_PORT Serial1 // HardwareSerial port pins 0 & 1
#define R_SENSE 0.11f

class PolarStepper {
private:
  TMC2209Stepper driver;
  int stepPin;
  int dirPin;

  int position = 0;
  int currentStep = 0;
  int maxSteps = 0;
  bool reversed = false;

  unsigned long nextStepTime;
  unsigned long nextStepTimeDelta;

public:
  PolarStepper(const int _driverAddress, const int _stepPin, const int _dirPin);
  void init();
  void begin();
  void setupMove(const int steps, const unsigned long currentMicros, const unsigned long stepTimeDelta);
  bool canMove();
  void move(const unsigned long currentMicros);
  int getCurrentStep();
  int getPosition();
  void setInterpolation(bool interpolate);
  void setMicrosteps(uint16_t microsteps);
  void printStatus();
};
