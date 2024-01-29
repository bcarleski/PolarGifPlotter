#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>
#define MAX_COMMANDS 2048

class DrawingClient {
private:
  HttpClient& client;
  const String& drawingsFile;
  const String& drawingPathPrefix;
  String drawing;
  String commands[MAX_COMMANDS];
  int commandCount;

  bool findNewDrawing();
  bool retrieveDrawing();

public:
  DrawingClient(HttpClient& client, const String& drawingsFile, const String& drawingPathPrefix);
  bool tryGetNewDrawing();
  String getDrawing();
  int getCommandCount();
  String getCommand(int index);
};
