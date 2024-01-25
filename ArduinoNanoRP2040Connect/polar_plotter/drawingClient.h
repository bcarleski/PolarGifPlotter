#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>

class DrawingClient {
private:
  HttpClient& client;
  const String& drawingsFile;
  const String& drawingPathPrefix;
  String drawing;
  JSONVar commandList;

  bool findNewDrawing();
  bool retrieveDrawing();

public:
  DrawingClient(HttpClient& client, const String& drawingsFile, const String& drawingPathPrefix);
  bool tryGetNewDrawing();
  String getDrawing();
  int getCommandCount();
  String getCommand(int index);
};
