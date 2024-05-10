#include "drawingProducer.h"

bool DrawingProducer::tryGetNewDrawing() {
  drawing = "UNKNOWN";

  String* drawingPointer = getNextDrawing();
  if (drawingPointer == NULL) {
    if (Serial) Serial.println("    Got NULL drawing.");
    return false;
  }

  String nextDrawing = *drawingPointer;
  if (Serial) Serial.println("    Got next drawing:\n    Body: " + nextDrawing);

  JSONVar json = JSON.parse(nextDrawing);
  if (!json.hasOwnProperty("commands")) {
    if (Serial) Serial.println("    Could not find commands node in drawing");
    return false;
  }

  JSONVar commandList = json["commands"];
  int cmdCount = commandList.length() > MAX_COMMANDS ? MAX_COMMANDS : commandList.length();
  commandCount = cmdCount;
  for (int i = 0; i < cmdCount; i++) {
    String command = commandList[i];
    commands[i] = command;
  }

  return commandCount > 0;
}

String DrawingProducer::getDrawing() {
  return drawing;
}

int DrawingProducer::getCommandCount() {
  return commandCount;
}

String DrawingProducer::getCommand(int index) {
  if (index < 0 || index >= commandCount) {
    String empty = "";
    return empty;
  }

  return commands[index];
}

