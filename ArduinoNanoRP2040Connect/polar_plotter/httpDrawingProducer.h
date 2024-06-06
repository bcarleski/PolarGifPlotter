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
  using DrawingProducer::setDrawingName;
  String* getNextDrawing() {
    printer.println("Trying to find new drawing in " + drawingsFile);

    client.stop();
    int error = client.get(drawingsFile);
    if (error != 0) {
      printer.print("    Got client error response of ");
      printer.println(error);
      return NULL;
    }

    int statusCode = client.responseStatusCode();
    String response = client.responseBody();
    printer.println("    Got response:\n        Body: " + response + "\n        Status Code: " + statusCode);

    deserializeJson(doc, response);
    if (!doc.containsKey("drawings")) {
      printer.println("    Could not find drawings node in response");
      return NULL;
    }

    JsonArrayConst drawings = doc["drawings"].as<JsonArrayConst>();
    if (drawings.size() > 0) {
      int index = random(drawings.size());

      JsonObjectConst entry = drawings[index].as<JsonObjectConst>();
      if (entry.containsKey("drawing")) {
        String drawingName = entry["drawing"].as<String>();
        doc.clear();

        setDrawingName(drawingName);

        String drawingPath = drawingPathPrefix + drawingName + ".json";
        printer.println("Trying to get new drawing from " + drawingPath);

        client.stop();
        int error = client.get(drawingPath);
        if (error != 0) {
          printer.print("    Got client error response of ");
          printer.println(error);
          return NULL;
        }

        int statusCode = client.responseStatusCode();
        String response = client.responseBody();
        printer.println("    Status Code: " + statusCode);

        return &response;
      }
    }

    doc.clear();

    printer.println("    Could not find a drawing in response");
    return NULL;
  }

public:
  HttpDrawingProducer(SafePrinter printer, HttpClient& client, const String& drawingsFile, const String& drawingPathPrefix)
    : client(client),
      drawingsFile(drawingsFile),
      drawingPathPrefix(drawingPathPrefix),
      DrawingProducer(printer) {}
  using DrawingProducer::tryGetNewDrawing;
  using DrawingProducer::getDrawing;
  using DrawingProducer::getCommandCount;
  using DrawingProducer::getCommand;
};
