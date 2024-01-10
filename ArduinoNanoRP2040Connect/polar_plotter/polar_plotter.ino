#include "lineSteps.h"

Point current;
LineSteps steps;
bool hasSteps = false;
unsigned int stepIndex = 0;
float radiusStepSize = 0.5;
float azimuthStepSize = PI / 360.0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');

    wipe();
    steps = *(new LineSteps(command, current, radiusStepSize, azimuthStepSize));
    hasSteps = true;
  }

  if (hasSteps) {
    if (stepIndex < steps.getStepCount()) {
      Step step = steps.getStep(stepIndex);
      stepIndex++;
      Serial.print(step.getRadiusStep());
      Serial.print(",");
      Serial.println(step.getAzimuthStep());
    } else {
      hasSteps = false;
    }
  }
}

void wipe() {
  Serial.println("WIPE");
  current.repoint(0, 0);
}