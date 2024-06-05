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

  deserializeJson(doc, nextDrawing);
  if (!doc.containsKey("commands")) {
    if (Serial) Serial.println("    Could not find commands node in drawing");
    return false;
  }

  JsonArrayConst commandList = doc["commands"].as<JsonArrayConst>();
  int cmdCount = commandList.size() > MAX_COMMANDS ? MAX_COMMANDS : commandList.size();
  commandCount = cmdCount;
  for (int i = 0; i < cmdCount; i++) {
    String command = commandList[i].as<String>();
    commands[i] = command;
  }

  doc.clear();

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

