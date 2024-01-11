#define RADIUS_STEP_POSITIVE 0x8
#define RADIUS_STEP_NEGATIVE 0x4
#define AZIMUTH_STEP_POSITIVE 0x2
#define AZIMUTH_STEP_NEGATIVE 0x1
#include "step.h"

Step::Step() {
  this->state = 0;
}

void Step::setSteps(int radiusStep, int azimuthStep) {
  byte state = 0;

  if (radiusStep > 0) {
    state |= RADIUS_STEP_POSITIVE;
  } else if (radiusStep < 0) {
    state |= RADIUS_STEP_NEGATIVE;
  }

  if (azimuthStep > 0) {
    state |= AZIMUTH_STEP_POSITIVE;
  } else if (azimuthStep < 0) {
    state |= AZIMUTH_STEP_NEGATIVE;
  }

  this->state = state;
}

int Step::getRadiusStep() {
  if (this->state & RADIUS_STEP_POSITIVE) {
    return 1;
  }

  if (this->state & RADIUS_STEP_NEGATIVE) {
    return -1;
  }

  return 0;
}

int Step::getAzimuthStep() {
  if (this->state & AZIMUTH_STEP_POSITIVE) {
    return 1;
  }

  if (this->state & AZIMUTH_STEP_NEGATIVE) {
    return -1;
  }

  return 0;
}