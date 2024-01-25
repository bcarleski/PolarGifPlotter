#include "drawingClient.h"

DrawingClient::DrawingClient(HttpClient& client, const String& drawingsFile, const String& drawingPathPrefix)
  : client(client),
    drawingsFile(drawingsFile),
    drawingPathPrefix(drawingPathPrefix) { }

bool DrawingClient::tryGetNewDrawing() {
  return this->findNewDrawing() && this->retrieveDrawing();
}

String DrawingClient::getDrawing() {
  return this->drawing;
}

int DrawingClient::getCommandCount() {
  return this->commandList.length();
}

String DrawingClient::getCommand(int index) {
  return this->commandList[index];
}

bool DrawingClient::findNewDrawing() {
  if (Serial) Serial.println("Trying to find new drawing in " + this->drawingsFile);

  this->client.stop();
  int error = this->client.get(this->drawingsFile);
  if (error != 0) {
    if (Serial) Serial.print("    Got client error response of ");
    if (Serial) Serial.println(error);
    return false;
  }

  int statusCode = this->client.responseStatusCode();
  String response = this->client.responseBody();
  if (Serial) Serial.println("    Got response:\n        Body: " + response + "\n        Status Code: " + statusCode);

  JSONVar json = JSON.parse(response);
  if (!json.hasOwnProperty("drawings")) {
    if (Serial) Serial.println("    Could not find drawings node in response");
    return false;
  }

  JSONVar drawings = json["drawings"];
  if (drawings.length() > 0) {
    int index = random(drawings.length());

    JSONVar entry = drawings[index];
    if (entry.hasOwnProperty("drawing")) {
      String drawing = entry["drawing"];
      this->drawing = drawing;
      return true;
    }
  }

  if (Serial) Serial.println("    Could not find a drawing in response");

  return false;
}

bool DrawingClient::retrieveDrawing() {
  String drawingPath = drawingPathPrefix + this->drawing + ".json";
  if (Serial) Serial.println("Trying to get new drawing from " + drawingPath);

  this->client.stop();
  int error = this->client.get(drawingPath);
  if (error != 0) {
    if (Serial) Serial.print("    Got client error response of ");
    if (Serial) Serial.println(error);
    return false;
  }

  int statusCode = this->client.responseStatusCode();
  String response = this->client.responseBody();
  if (Serial) Serial.println("    Got response:\n    Body: " + response + "\n    Status Code: " + statusCode);

  JSONVar json = JSON.parse(response);
  if (!json.hasOwnProperty("commands")) {
    if (Serial) Serial.println("    Could not find commands node in response");
    return false;
  }

  this->commandList = json["commands"];

  return this->commandList.length() > 0;
}
