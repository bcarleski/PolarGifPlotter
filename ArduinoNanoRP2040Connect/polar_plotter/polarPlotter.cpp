#include "polarPlotter.h"
#define MARBLE_SIZE_IN_RADIUS_STEPS 20

PolarPlotter::PolarPlotter(CondOut& condOut, Stepper& radiusStepper, Stepper& azimuthStepper, float maxRadius, float radiusStepSize, float azimuthStepSize)
  : condOut(condOut),
    radiusStepper(radiusStepper),
    azimuthStepper(azimuthStepper),
    maxRadius(maxRadius),
    radiusStepSize(radiusStepSize),
    azimuthStepSize(azimuthStepSize),
    steps(StepBank(condOut)),
    lineStepCalculator(LineStepCalculator(condOut, radiusStepSize, azimuthStepSize)) {
}

void PolarPlotter::init(float startingRadius, float startingAzimuth) {
  this->finish.repoint(startingRadius, startingAzimuth);
  this->position.cloneFrom(this->finish);
  this->radiusStepper.setSpeed(RADIUS_RPMS);
  this->azimuthStepper.setSpeed(AZIMUTH_RPMS);
}

void PolarPlotter::computeSteps(String& command) {
  unsigned long startMillis = millis();
  this->stepIndex = 0;
  this->steps.reset();

  if (this->debugLevel >= 1) {
    this->condOut.print(" COMMAND - ");
    this->condOut.println(command);
  }

  this->condOut.clear();
  this->condOut.setCursor(0, 0);
  this->condOut.lcdPrint(command);

  switch (command.charAt(0)) {
    case 'l':
      setFinishPoint(command);
      this->lineStepCalculator.addLineSteps(this->position, this->finish, this->steps);
      break;
  }

  unsigned long endMillis = millis();
  long duration = endMillis - startMillis;
  this->condOut.setCursor(0, 1);
  this->condOut.lcdPrint("C:");
  this->condOut.lcdPrint(duration);

  if (this->debugLevel >= 1) {
    this->condOut.print(" DURATION - ");
    this->condOut.println(duration);
  }
}

void PolarPlotter::executeStepsToCenter() {
  float radius = this->position.getRadius();
  float azimuth = this->position.getAzimuth();

  if (abs(radius - 0) < 0.0000001 && abs(azimuth - 0) < 0.0000001) {
    return;
  }

  int radiusOffset = radius > 0 ? -1 : 1;
  int azimuthOffset = azimuth > 0 ? -1 : 1;
  int radiusSteps = round(radius / this->radiusStepSize);
  int azimuthSteps = round(azimuth / this->azimuthStepSize);
  int max = radiusSteps > azimuthSteps ? radiusSteps : azimuthSteps;

  for (int i = 0; i < max; i++) {
    this->steps.addStep(radiusSteps > i ? radiusOffset : 0, azimuthSteps > i ? azimuthOffset : 0);
  }

  while (this->step())
    ;

  this->stepIndex = 0;
  this->steps.reset();
}

void PolarPlotter::executeRadiusSteps(int radiusSteps) {
  for (int i = 0; i < radiusSteps; i++) {
    this->steps.addStep(1, 0);
  }

  while (this->step())
    ;

  this->stepIndex = 0;
  this->steps.reset();
}

void PolarPlotter::executeFullCircleSteps() {
  int azimuthSteps = round(2 * PI / this->azimuthStepSize);

  for (int i = 0; i < azimuthSteps; i++) {
    this->steps.addStep(0, 1);
  }

  while (this->step())
    ;

  this->stepIndex = 0;
  this->steps.reset();
}

void PolarPlotter::executeWipe() {
  this->stepIndex = 0;
  this->steps.reset();

  this->condOut.print(" WIPING");
  this->condOut.lcdPrint("WIPING", "");

  float radiusSteps = this->maxRadius / this->radiusStepSize;
  this->executeStepsToCenter();
  this->executeRadiusSteps(ceil(radiusSteps));
  this->executeFullCircleSteps();

  int stepChunks = floor(radiusSteps / MARBLE_SIZE_IN_RADIUS_STEPS);
  int remainder = ceil(radiusSteps - (stepChunks * MARBLE_SIZE_IN_RADIUS_STEPS));

  for (int i = 0; i < stepChunks; i++) {
    for (int j = 0; j < MARBLE_SIZE_IN_RADIUS_STEPS; j++) {
      this->steps.addStep(-1, 0);
    }

    this->executeFullCircleSteps();
  }

  if (remainder > 0) {
    this->executeRadiusSteps(remainder);
  }

  this->position.repoint(0, 0);
  this->finish.repoint(0, 0);
}

bool PolarPlotter::step() {
  if (this->stepIndex < 0 || this->stepIndex >= this->steps.getStepCount()) {
    unsigned long endMillis = micros();
    long duration = endMillis - this->stepStartMillis;
    this->condOut.setCursor(8, 1);
    this->condOut.lcdPrint("S:");
    this->condOut.lcdPrint(duration);
    this->stepStartMillis = 0;

    if (this->debugLevel >= 1) {
      this->condOut.print(" STEPPING DURATION - ");
      this->condOut.println(duration);
    }

    this->position.cloneFrom(this->finish);
    return false;
  }

  if (this->stepStartMillis == 0) {
    this->stepStartMillis = micros();
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
  int azimuthStep = step.getAzimuthStep();

  if (newRadius >= this->maxRadius) {
    radiusStep = 0;
  }

  this->condOut.print("STEP: ");
  this->condOut.print(radiusStep);
  this->condOut.print(",");
  this->condOut.println(azimuthStep);
  if (radiusStep != 0) {
    this->radiusStepper.step(radiusStep);
  }
  if (azimuthStep != 0) {
    this->azimuthStepper.step(azimuthStep);
  }
  this->position.repoint(newRadius, newAzimuth);
  this->stepIndex++;

  if (this->debugLevel >= 2) {
    pos = this->position;
    this->condOut.print(" Position", pos);
    this->condOut.print(", from=(");
    this->condOut.print(oldRadius, 4);
    this->condOut.print(", ");
    this->condOut.print(oldAzimuth, 4);
    this->condOut.print("), delta=(");
    this->condOut.print(newRadiusDelta, 4);
    this->condOut.print(", ");
    this->condOut.print(newAzimuthDelta, 4);
    this->condOut.print("), to=(");
    this->condOut.print(newRadius, 4);
    this->condOut.print(", ");
    this->condOut.print(newAzimuth, 4);
    this->condOut.println(")");
  }

  return true;
}

unsigned int PolarPlotter::getStepCount() {
  return this->steps.getStepCount();
}

Point PolarPlotter::getPosition() {
  return this->position;
}

void PolarPlotter::setDebug(unsigned int level) {
  this->debugLevel = level;
  this->lineStepCalculator.setDebug(level);
  this->steps.setDebug(level);
  if (level > 0) {
    String msg = " Set debug level to ";
    this->condOut.println(msg + level);
  }
}

String PolarPlotter::getHelpMessage() {
  return "help       This help message\n"
         "W          Wipe any existing drawing\n"
         "L{X},{Y}   Draw a line from the origin to the cartesian point (X,Y)\n"
         "D{#}       Set the debug level between 0-9 (0-Off, 9-Most Verbose)";
}
