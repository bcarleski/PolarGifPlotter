#include "stepBank.h"

StepBank::StepBank(CondOut& condOut) : condOut(condOut) {
}

void StepBank::addStep(int radiusStep, int azimuthStep) {
  if (this->stepCount >= MAX_STEPS) {
    this->stepCount = 0;
  }

  this->steps[this->stepCount++].setSteps(radiusStep, azimuthStep);
}

Step StepBank::getStep(unsigned int stepIndex) {
  if (this->debugLevel >= 5) {
    condOut.print("     Getting Step ");
    condOut.print(stepIndex);
    condOut.print(" of ");
    condOut.println(this->stepCount);
  }
  if (stepIndex < 0 || stepIndex >= this->stepCount) {
    return this->blankStep;
  }

  return this->steps[stepIndex];
}

unsigned int StepBank::getStepCount() {
  return this->stepCount;
}

void StepBank::reset() {
  this->stepCount = 0;
}

void StepBank::setDebug(unsigned int level) {
  this->debugLevel = level;
}
