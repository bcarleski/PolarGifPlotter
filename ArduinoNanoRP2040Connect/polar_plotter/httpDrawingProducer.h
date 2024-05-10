#include <ArduinoHttpClient.h>
#include "drawingProducer.h"

class HttpDrawingProducer : public DrawingProducer {
private:
  HttpClient& client;
  const String& drawingsFile;
  const String& drawingPathPrefix;

  bool findNewDrawing();
  bool retrieveDrawing();

protected:
  String* getNextDrawing();
  using DrawingProducer::setDrawingName;

public:
  HttpDrawingProducer(HttpClient& client, const String& drawingsFile, const String& drawingPathPrefix);
  using DrawingProducer::tryGetNewDrawing;
  using DrawingProducer::getDrawing;
  using DrawingProducer::getCommandCount;
  using DrawingProducer::getCommand;
};
