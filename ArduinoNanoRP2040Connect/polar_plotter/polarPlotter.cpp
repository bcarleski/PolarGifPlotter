#include "polarPlotter.h"

PolarPlotter::PolarPlotter() {
}

void PolarPlotter::init(float maxRadius, float radiusStepSize, float azimuthStepSize) {
  this->maxRadius = maxRadius;
  this->radiusStepSize = radiusStepSize;
  this->azimuthStepSize = azimuthStepSize;
  this->lineStepCalculator.init(radiusStepSize, azimuthStepSize);
}

void PolarPlotter::computeSteps(String& command) {
  this->stepIndex = 0;
  this->steps.reset();

  if (this->debugLevel >= 1) {
    Serial.print(" COMMAND - ");
    Serial.println(command);
  }

  switch (command.charAt(0)) {
    case 'l':
      setFinishPoint(command);
      this->lineStepCalculator.addLineSteps(this->position, this->finish, this->steps);
      break;
    case 'w':
      this->position.repoint(0, 0);
      break;
  }
}

bool PolarPlotter::step() {
  if (this->stepIndex < 0 || this->stepIndex >= this->steps.getStepCount()) {
    this->position.cloneFrom(this->finish);
    return false;
  }

  Step step = steps.getStep(this->stepIndex);
  Point pos = this->position;
  float oldRadius = pos.getRadius();
  float oldAzimuth = pos.getAzimuth();
  float newRadiusDelta = step.getRadiusStep() * this->radiusStepSize;
  float newAzimuthDelta = step.getAzimuthStep() * this->azimuthStepSize;
  float newRadius = oldRadius + newRadiusDelta;
  float newAzimuth = oldAzimuth + newAzimuthDelta;
  int radiusStep = step.getRadiusStep();

  if (newRadius >= this->maxRadius) {
    newRadius = oldRadius;
    newRadiusDelta = 0;
    radiusStep = 0;
  }

  this->position.repoint(newRadius, newAzimuth);
  this->stepIndex++;
  Serial.print(radiusStep);
  Serial.print(",");
  Serial.println(step.getAzimuthStep());

  if (this->debugLevel >= 1) {
    pos = this->position;
    Serial.print(" Position=(");
    Serial.print(pos.getX(), 4);
    Serial.print(", ");
    Serial.print(pos.getY(), 4);
    Serial.print(", ");
    Serial.print(pos.getRadius(), 4);
    Serial.print(", ");
    Serial.print(pos.getAzimuth(), 4);
    Serial.print("), from=(");
    Serial.print(oldRadius, 4);
    Serial.print(", ");
    Serial.print(oldAzimuth, 4);
    Serial.print("), delta=(");
    Serial.print(newRadiusDelta, 4);
    Serial.print(", ");
    Serial.print(newAzimuthDelta, 4);
    Serial.print("), to=(");
    Serial.print(newRadius, 4);
    Serial.print(", ");
    Serial.print(newAzimuth, 4);
    Serial.println(")");
  }
}

void PolarPlotter::setDebug(unsigned int level) {
  this->debugLevel = level;
  this->lineStepCalculator.setDebug(level);
}

String PolarPlotter::getHelpMessage() {
  return "help       This help message\n"
         "W          Wipe any existing drawing\n"
         "L{X},{Y}   Draw a line from the origin to the cartesian point (X,Y)\n"
         "D{#}       Set the debug level between 0-9 (0-Off, 9-Most Verbose)";
}
