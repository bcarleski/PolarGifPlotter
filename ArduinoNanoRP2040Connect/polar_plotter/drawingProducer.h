#include <ArduinoJson.h>
#define MAX_COMMANDS 2048

class DrawingProducer {
private:
  String drawing;
  String commands[MAX_COMMANDS];
  int commandCount;

protected:
  JsonDocument doc;
  virtual String* getNextDrawing() = 0;
  void setDrawingName(String& name) { drawing = name; }

public:
  bool tryGetNewDrawing();
  String getDrawing();
  int getCommandCount();
  String getCommand(int index);
};
