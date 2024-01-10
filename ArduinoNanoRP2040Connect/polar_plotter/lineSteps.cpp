#include "lineSteps.h"

LineSteps::LineSteps() {
  this->stepCount = 0;
}

LineSteps::LineSteps(String& command, Point& current, float radiusStepSize, float azimuthStepSize) {
  this->radiusStepSize = radiusStepSize;
  this->azimuthStepSize = azimuthStepSize;
  this->start = current;
  this->finish = *(getCommandPoint(command));

  float xDelta = this->finish.getX() - this->start.getX();
  float yDelta = this->finish.getY() - this->start.getY();
  this->xDelta = xDelta;
  this->yDelta = yDelta;
  this->distanceDenominator = sqrt(xDelta * xDelta + yDelta * yDelta);
  this->stepCount = 0;

  if (command.charAt(0) == 'L') {
    addLineSteps();
  }
}

Step LineSteps::getStep(unsigned int stepIndex) {
  if (stepIndex < 0 || stepIndex >= this->stepCount) {
    return *(new Step());
  }

  return this->steps[stepIndex];
}

unsigned int LineSteps::getStepCount() {
  return this->stepCount;
}