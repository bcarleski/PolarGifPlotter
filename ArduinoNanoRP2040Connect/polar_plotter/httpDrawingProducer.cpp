#include "httpDrawingProducer.h"

HttpDrawingProducer::HttpDrawingProducer(HttpClient& client, const String& drawingsFile, const String& drawingPathPrefix)
  : client(client),
    drawingsFile(drawingsFile),
    drawingPathPrefix(drawingPathPrefix) { }

String* HttpDrawingProducer::getNextDrawing() {
  if (Serial) Serial.println("Trying to find new drawing in " + drawingsFile);

  client.stop();
  int error = client.get(drawingsFile);
  if (error != 0) {
    if (Serial) Serial.print("    Got client error response of ");
    if (Serial) Serial.println(error);
    return NULL;
  }

  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  if (Serial) Serial.println("    Got response:\n        Body: " + response + "\n        Status Code: " + statusCode);

  deserializeJson(doc, response);
  if (!doc.containsKey("drawings")) {
    if (Serial) Serial.println("    Could not find drawings node in response");
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
      if (Serial) Serial.println("Trying to get new drawing from " + drawingPath);

      client.stop();
      int error = client.get(drawingPath);
      if (error != 0) {
        if (Serial) Serial.print("    Got client error response of ");
        if (Serial) Serial.println(error);
        return NULL;
      }

      int statusCode = client.responseStatusCode();
      String response = client.responseBody();
      if (Serial) Serial.println("    Status Code: " + statusCode);

      return &response;
    }
  }

  doc.clear();

  if (Serial) Serial.println("    Could not find a drawing in response");
  return NULL;
}
