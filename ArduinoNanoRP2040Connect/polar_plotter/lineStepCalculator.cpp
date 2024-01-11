#include "lineStepCalculator.h"

LineStepCalculator::LineStepCalculator() {
}

void LineStepCalculator::init(float radiusStepSize, float azimuthStepSize) {
  this->radiusStepSize = radiusStepSize;
  this->azimuthStepSize = azimuthStepSize;
}

void LineStepCalculator::addLineSteps(Point& start, Point& finish, StepBank& steps) {
  orientEndPoint(start, finish);

  float startA = start.getAzimuth();
  float radiusStepOffset = finish.getRadius() > start.getRadius() ? this->radiusStepSize : (-1 * this->radiusStepSize);
  float azimuthStepOffset = finish.getAzimuth() > startA ? this->azimuthStepSize : (-1 * this->azimuthStepSize);
  int radiusSteps = round(abs(finish.getRadius() - start.getRadius()) / this->radiusStepSize);
  int azimuthSteps = round(abs(finish.getAzimuth() - startA) / this->azimuthStepSize);

  if (this->debugLevel >= 1) {
    Serial.print(" Going from (");
    Serial.print(start.getX(), 4);
    Serial.print(",");
    Serial.print(start.getY(), 4);
    Serial.print(") to (");
    Serial.print(finish.getX(), 4);
    Serial.print(",");
    Serial.print(finish.getY(), 4);
    Serial.print(") in ");
    Serial.print(radiusSteps);
    Serial.print(" radius steps and ");
    Serial.print(azimuthSteps);
    Serial.print(" azimuth steps with a radius step offset of ");
    Serial.print(radiusStepOffset, 4);
    Serial.print(" and an azimuth step offset of ");
    Serial.println(azimuthStepOffset, 4);
  }

  // If we are at the origin, make sure we are pointed in the right direction before heading out
  if (azimuthSteps > 0 && abs(start.getRadius()) <= (this->radiusStepSize * 0.1)) {
    addBulkSteps(steps, 0, azimuthStepOffset, azimuthSteps);
    startA = finish.getAzimuth();
    azimuthSteps = 0;
  }

  // If we are pointed in the right direction, just push in/out to where we need to go
  if (radiusSteps > 0 && abs(finish.getAzimuth() - startA) <= (this->azimuthStepSize * 0.1)) {
    addBulkSteps(steps, radiusStepOffset, 0, radiusSteps);
    return;
  }

  if (radiusSteps == 0 && azimuthSteps == 0) {
    return;
  }

  // Draw the line in two section, to better handle line drawing
  this->middle.cartesianRepoint((start.getX() + finish.getX()) / 2, (start.getY() + finish.getY()) / 2);
  orientEndPoint(start, this->middle);
  addCompoundStepsForLine(steps, start, this->middle, startA);
  addCompoundStepsForLine(steps, this->middle, finish, this->middle.getAzimuth());
}

void LineStepCalculator::setDebug(unsigned int level) {
  this->debugLevel = level;
}
