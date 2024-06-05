#include "PolarStepper.h"

PolarStepper::PolarStepper(const int _driverAddress, const int _stepPin, const int _dirPin)
  : driver(TMC2209Stepper(&SERIAL_PORT, R_SENSE, _driverAddress)),
    stepPin(_stepPin),
    dirPin(_dirPin)
{
}

void PolarStepper::init() {
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

void PolarStepper::begin() {
  digitalWrite(stepPin, LOW);
  digitalWrite(dirPin, LOW);
  driver.begin();
  driver.reset();
  driver.toff(5);
  driver.microsteps(0);
  driver.intpol(true);
}

void PolarStepper::setupMove(const int steps, const unsigned long currentMicros, const unsigned long stepTimeDelta) {
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

bool PolarStepper::canMove() {
  return currentStep < maxSteps;
}

void PolarStepper::move(const unsigned long currentMicros) {
  if (!canMove() || currentMicros < nextStepTime) return;

  nextStepTime += nextStepTimeDelta;
  currentStep++;
  position += (reversed ? -1 : 1);

  digitalWrite(stepPin, HIGH);
  delayMicroseconds(2);
  digitalWrite(stepPin, LOW);
}

void PolarStepper::declareOrigin() {
  position = 0;
}

int PolarStepper::getCurrentStep() {
  return currentStep * (reversed ? -1 : 1);
}

int PolarStepper::getPosition() {
  return position;
}
