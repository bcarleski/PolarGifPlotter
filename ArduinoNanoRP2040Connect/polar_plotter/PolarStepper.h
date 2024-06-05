#include <TMCStepper.h>

#define SERIAL_PORT Serial1  // HardwareSerial port pins 0 & 1
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
  PolarStepper(const int _driverAddress, const int _stepPin, const int _dirPin)
    : driver(TMC2209Stepper(&SERIAL_PORT, R_SENSE, _driverAddress)),
      stepPin(_stepPin),
      dirPin(_dirPin) {}
  void init() {
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
  }
  void begin() {
    digitalWrite(stepPin, LOW);
    digitalWrite(dirPin, LOW);
    driver.begin();
    driver.reset();
    driver.toff(5);
    driver.microsteps(0);
    driver.intpol(true);
  }
  void setupMove(const int steps, const unsigned long currentMicros, const unsigned long stepTimeDelta) {
    currentStep = 0;
    maxSteps = abs(steps);
    nextStepTimeDelta = stepTimeDelta;
    nextStepTime = currentMicros + stepTimeDelta;

    if ((steps < 0) != reversed) {
      reversed = steps < 0;
      digitalWrite(dirPin, reversed ? HIGH : LOW);
      delayMicroseconds(20);
    }
  }
  bool canMove() {
    return currentStep < maxSteps;
  }
  void move(const unsigned long currentMicros) {
    if (!canMove() || currentMicros < nextStepTime) return;

    nextStepTime += nextStepTimeDelta;
    currentStep++;
    position += (reversed ? -1 : 1);

    digitalWrite(stepPin, HIGH);
    delayMicroseconds(2);
    digitalWrite(stepPin, LOW);
  }
  void declareOrigin() {
    position = 0;
  }
  int getCurrentStep() {
    return currentStep * (reversed ? -1 : 1);
  }
  int getPosition() {
    return position;
  }
};
