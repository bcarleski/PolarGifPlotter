#include <ArduinoJson.h>
#define MAX_COMMANDS 2048
const int KNOWN_DRAWINGS_COUNT = 8;
const String KNOWN_DRAWINGS[] = {
  "{\"commands\":[\"L600,600\",\"L600,-600\",\"L-600,-600\",\"L-600,600\",\"L600,600\"]}",
  "{\"commands\":[\"L0,680\",\"L-480,-680\",\"L720,280\",\"L-720,280\",\"L480,-680\",\"L0,680\"]}",
  "{\"commands\":[\"L0,400\",\"L-208,544\",\"L-480,512\",\"L-536,456\",\"L-608,392\",\"L-640,224\",\"L-640,144\",\"L-544,0\",\"L0,-512\",\"L544,0\",\"L640,144\",\"L640,224\",\"L608,392\",\"L536,456\",\"L480,512\",\"L208,544\",\"L0,400\"]}",
  "{\"commands\":[\"L0,400\",\"C-272,200,180\",\"L0,-512\",\"L544,0\",\"C272,200,180\"]}",
  "{\"commands\":[\"S0,90\",\"S900,90\",\"S0,540\",\"S-900,-90\"]}",
  "{\"commands\":[\"C-500,0,180\",\"S0,540\",\"C500,0,-180\",\"L-250,0\",\"C-500,0,180\",\"C-500,0,180\",\"L250,0\",\"C500,0,180\",\"C500,0,180\"]}",
  "{\"commands\":[\"C0,600,90\",\"C-600,0,180\",\"C0,-600,-180\",\"C600,0,180\",\"C0,600,-90\",\"L-600,600\",\"L600,600\",\"L-600,-600\",\"L600,-600\",\"L0,0\",\"L848.528,0\",\"S0,540\",\"L0,0\"]}",
  "{\"commands\":[\"S900,360\",\"S0,540\",\"S-900,-360\",\"S0,90\",\"S900,360\",\"S0,180\",\"S-900,-360\"]}"
};

class DrawingProducer {
private:
  String drawing;
  String commands[MAX_COMMANDS];
  int commandCount;

protected:
  JsonDocument doc;
  virtual String* getNextDrawing() = 0;
  void setDrawingName(String& name) {
    drawing = name;
  }

public:
  bool tryGetNewDrawing() {
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
  String getDrawing() {
    return drawing;
  }
  int getCommandCount() {
    return commandCount;
  }
  String getCommand(int index) {
    if (index < 0 || index >= commandCount) {
      String empty = "";
      return empty;
    }

    return commands[index];
  }
};


class KnownDrawingProducer : public DrawingProducer {
private:
  String nextDrawing;

protected:
  String* getNextDrawing() {
    int idx = random(0, KNOWN_DRAWINGS_COUNT);
    String drawingName = "Drawing_" + String(idx + 1);
    setDrawingName(drawingName);

    nextDrawing = KNOWN_DRAWINGS[idx];
    return &nextDrawing;
  }
  using DrawingProducer::setDrawingName;

public:
  using DrawingProducer::tryGetNewDrawing;
  using DrawingProducer::getDrawing;
  using DrawingProducer::getCommandCount;
  using DrawingProducer::getCommand;
};