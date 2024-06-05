#include "PolarCoordinateStepper.h"

PolarCoordinateStepper::PolarCoordinateStepper(const int _radiusDriverAddress, const int _radiusStepPin, const int _radiusDirPin,
                                               const int _azimuthDriverAddress, const int _azimuthStepPin, const int _azimuthDirPin,
                                               const int _interlocked)
  : radius(PolarStepper(_radiusDriverAddress, _radiusStepPin, _radiusDirPin)),
    azimuth(PolarStepper(_azimuthDriverAddress, _azimuthStepPin, _azimuthDirPin)),
    interlocked(_interlocked)
{
}

void PolarCoordinateStepper::init() {
  radius.init();
  azimuth.init();

  SERIAL_PORT.begin(115200);
}

void PolarCoordinateStepper::begin() {
  radius.begin();
  azimuth.begin();
}

bool PolarCoordinateStepper::canAddSteps() {
  return savingIndex != movingIndex;
}

void PolarCoordinateStepper::changeSpeed(const unsigned long offset) {
  if (offset >= LOWEST_MINIMUM_OFFSET && offset < 1000000000UL && offset != minimumOffset) {
    minimumOffset = offset;
    recalculateMove();
  }
}

void PolarCoordinateStepper::addSteps(const int radiusStep, const int azimuthStep, const bool fastStep) {
  if (!canAddSteps()) return;

  radiusSteps[savingIndex] = radiusStep;
  azimuthSteps[savingIndex] = azimuthStep;
  savingIndex = getNextIndex(savingIndex);
}

void PolarCoordinateStepper::declareOrigin() {
  radius.declareOrigin();
  azimuth.declareOrigin();
}

void PolarCoordinateStepper::move() {
  if (paused) return;
  if (!radius.canMove() && !azimuth.canMove() && !prepareMove()) {
    if (moving) moving = false;
    return;
  }

  if (!moving) moving = true;
  unsigned long currentMicros = micros();
  radius.move(currentMicros);
  azimuth.move(currentMicros);
}

void PolarCoordinateStepper::reset() {
  setupMove(radius.getPosition() * -1, azimuth.getPosition() * -1, true);
}

void PolarCoordinateStepper::pause() {
  paused = true;
}

void PolarCoordinateStepper::resume() {
  paused = false;
  recalculateMove();
}

void PolarCoordinateStepper::stop() {
  radius.setupMove(0, 0, 0);
  azimuth.setupMove(0, 0, 0);

  int nextRadiusSteps = radiusSteps[movingIndex];
  int nextAzimuthSteps = azimuthSteps[movingIndex];
}


int PolarCoordinateStepper::getNextIndex(int index) {
  return (index + 1) % MAX_POLAR_STEPS;
}

bool PolarCoordinateStepper::hasSteps() {
  return getNextIndex(movingIndex) != savingIndex;
}

bool PolarCoordinateStepper::prepareMove() {
  if (!hasSteps()) return false;

  movingIndex = getNextIndex(movingIndex);
  int nextRadiusSteps = radiusSteps[movingIndex];
  int nextAzimuthSteps = azimuthSteps[movingIndex];
  bool fastStep = fastSteps[movingIndex];

  return setupMove(nextRadiusSteps, nextAzimuthSteps, fastStep);
}

void PolarCoordinateStepper::recalculateMove() {
  if (!moving) return;

  int nextRadiusSteps = radiusSteps[movingIndex] - radius.getCurrentStep();
  int nextAzimuthSteps = azimuthSteps[movingIndex] - azimuth.getCurrentStep();
  bool fastStep = fastSteps[movingIndex];

  setupMove(nextRadiusSteps, nextAzimuthSteps, fastStep);
}

bool PolarCoordinateStepper::setupMove(const int nextRadiusSteps, const int nextAzimuthSteps, const bool fastStep) {
  int rSteps = abs(nextRadiusSteps);
  int aSteps = abs(nextAzimuthSteps);
  int maxSteps = rSteps > aSteps ? rSteps : aSteps;

  if (maxSteps == 0) return false;

  unsigned long moveTime = maxSteps * minimumOffset * (fastStep ? 1 : SLOW_STEP_MULTIPLIER);
  unsigned long radiusStepTimeDelta = moveTime / rSteps;
  unsigned long azimuthStepTimeDelta = moveTime / aSteps;
  unsigned long currentMicros = micros();

  radius.setupMove(nextRadiusSteps, currentMicros, radiusStepTimeDelta);
  azimuth.setupMove(nextAzimuthSteps, currentMicros, azimuthStepTimeDelta);
}
