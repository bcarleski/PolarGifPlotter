#include "lineStepCalculator.h"

LineStepCalculator::LineStepCalculator(CondOut& condOut, float radiusStepSize, float azimuthStepSize)
  : condOut(condOut),
    radiusStepSize(radiusStepSize),
    azimuthStepSize(azimuthStepSize)
{
}

void LineStepCalculator::addLineSteps(Point& start, Point& finish, StepBank& steps) {
  orientEndPoint(start, finish);

  float startA = start.getAzimuth();
  float radiusStepOffset = finish.getRadius() > start.getRadius() ? this->radiusStepSize : (-1 * this->radiusStepSize);
  float azimuthStepOffset = finish.getAzimuth() > startA ? this->azimuthStepSize : (-1 * this->azimuthStepSize);
  int radiusSteps = round(abs(finish.getRadius() - start.getRadius()) / this->radiusStepSize);
  int azimuthSteps = round(abs(finish.getAzimuth() - startA) / this->azimuthStepSize);

  if (this->debugLevel >= 2) {
    this->condOut.print(" Going from (");
    this->condOut.print(start.getX(), 4);
    this->condOut.print(",");
    this->condOut.print(start.getY(), 4);
    this->condOut.print(",");
    this->condOut.print(start.getRadius(), 4);
    this->condOut.print(",");
    this->condOut.print(start.getAzimuth(), 4);
    this->condOut.print(") to (");
    this->condOut.print(finish.getX(), 4);
    this->condOut.print(",");
    this->condOut.print(finish.getY(), 4);
    this->condOut.print(",");
    this->condOut.print(finish.getRadius(), 4);
    this->condOut.print(",");
    this->condOut.print(finish.getAzimuth(), 4);
    this->condOut.print(") in ");
    this->condOut.print(radiusSteps);
    this->condOut.print(" radius steps and ");
    this->condOut.print(azimuthSteps);
    this->condOut.print(" azimuth steps with a radius step offset of ");
    this->condOut.print(radiusStepOffset, 4);
    this->condOut.print(" and an azimuth step offset of ");
    this->condOut.println(azimuthStepOffset, 4);
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
