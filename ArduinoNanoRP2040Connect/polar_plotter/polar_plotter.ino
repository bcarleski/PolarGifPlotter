#include "polarPlotter.h"

PolarPlotter plotter;
bool hasSteps = false;
float maxRadius = 600;
float radiusStepSize = 1;
float azimuthStepSize = PI / 360.0;

void setup() {
  Serial.begin(9600);
  plotter.init(maxRadius, radiusStepSize, azimuthStepSize);
}

void loop() {
  if (Serial.available()) {
    handleInput();
  }

  if (hasSteps) {
    hasSteps = plotter.step();
  }
}

void handleInput() {
  String command = Serial.readStringUntil('\n');
  command.toLowerCase();

  if (command == "help" || command == "--help" || command == "-help" || command == "/help" || command == "-h" || command == "/h" || command == "-?" || command == "/?") {
    String helpMessage = PolarPlotter::getHelpMessage();
    Serial.println(helpMessage);
  } else if (command.startsWith("d")) {
    unsigned int debugLevel = (unsigned int)command.substring(1).toInt();
    plotter.setDebug(debugLevel);
  } else {
    plotter.computeSteps(command);
    hasSteps = true;
  }
}
