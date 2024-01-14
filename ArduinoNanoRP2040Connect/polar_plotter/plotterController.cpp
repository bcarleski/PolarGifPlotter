#include "plotterController.h"

PlotterController::PlotterController(PolarPlotter& plotter, const String& deviceName, const String& shadowName)
  : plotter(plotter),
    getTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/get"),
    getAcceptedTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/get/accepted"),
    getRejectedTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/get/rejected"),
    updateTopic("$aws/things/" + deviceName + "/shadow/name/" + shadowName + "/update"),
    toDeviceTopic(deviceName + "/toDevice"),
    fromDeviceTopic(deviceName + "/fromDevice"),
    currentDrawing("") {
  this->topics[0] = this->getAcceptedTopic;
  this->topics[1] = this->getRejectedTopic;
  this->topics[2] = this->toDeviceTopic;
}

void PlotterController::onMessage(void publishMessage(const String&, const JSONVar&)) {
  this->publishMessage = publishMessage;
}

void PlotterController::onMqttPoll(bool pollMqtt(String[])) {
  this->pollMqtt = pollMqtt;
}

void PlotterController::performCycle() {
  if (Serial && Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.toLowerCase();

    if (command.startsWith("d")) {
      unsigned int debugLevel = (unsigned int)command.substring(1).toInt();
      this->plotter.setDebug(debugLevel);
    } else if (command == "w") {
      this->plotter.executeWipe();
    } else {
      this->plotter.computeSteps(command);
      this->hasSteps = this->plotter.getStepCount() > 0;
    }
  }

  if (this->hasSteps) {
    this->hasSteps = plotter.step();
  }

  if (!this->pollMqtt(this->topics) || this->waitingForDeviceShadow || this->waitingForDrawing || this->waitingForLine) {
    return;
  }

  if (!this->initialized) {
    JSONVar empty;
    this->waitingForDeviceShadow = true;
    this->publishMessage(this->getTopic, empty);
    this->initialized = true;
    return;
  }

  if (this->hasSteps) {
    return;
  }

  if (this->currentDrawing == "" || this->currentLine >= this->totalLines) {
    this->requestDrawing();
  } else {
    this->requestLine();
  }
}

void PlotterController::messageReceived(const String& topic, const String& message) {
  if (topic == this->getAcceptedTopic) {
    this->getAcceptedMessage(message);
  } else if (topic == this->getRejectedTopic) {
    this->getRejectedMessage(message);
  } else if (topic == this->toDeviceTopic) {
    this->toDeviceMessage(message);
  }
}

void PlotterController::requestDrawing() {
  this->waitingForDrawing = true;
  this->currentDrawing = "";
  this->currentLine = -1;

  JSONVar message;
  message["command"] = "drawing";

  this->publishMessage(this->fromDeviceTopic, message);
}

void PlotterController::requestLine() {
  this->waitingForLine = true;
  this->currentLine++;

  JSONVar message;
  message["command"] = "line";
  message["drawing"] = this->currentDrawing;
  message["line"] = this->currentLine;

  this->publishMessage(this->fromDeviceTopic, message);
}

void PlotterController::getAcceptedMessage(const String& message) {
  this->waitingForDeviceShadow = false;

  JSONVar json = JSON.parse(message);
  if (!json.hasOwnProperty("state")) {
    reportError("Missing getAccepted.state", message);
    return;
  }

  JSONVar state = json["state"];
  if (!state.hasOwnProperty("reported")) {
    reportError("Missing getAccepted.state.reported", message);
    return;
  }

  JSONVar reported = state["reported"];
  if (!reported.hasOwnProperty("drawing") || !reported.hasOwnProperty("totalLines")) {
    // This isn't an error, there just isn't any state we can load
    return;
  }

  String drawing = reported["drawing"];
  int totalLines = reported["totalLines"];

  this->currentDrawing = drawing;
  this->totalLines = totalLines;

  if (reported.hasOwnProperty("line")) {
    int line = reported["line"];
    this->currentLine = line;
  }

  if (reported.hasOwnProperty("radius") && reported.hasOwnProperty("azimuth")) {
    double radius = reported["radius"];
    double azimuth = reported["azimuth"];

    this->plotter.init(radius, azimuth);
  }
}

void PlotterController::getRejectedMessage(const String& message) {
  this->waitingForDeviceShadow = false;
  this->plotter.executeWipe();
}

void PlotterController::toDeviceMessage(const String& message) {
  JSONVar json = JSON.parse(message);
  bool handled = false;

  if (json.hasOwnProperty("type")) {
    String type = json["type"];

    if ((this->waitingForDrawing && type == "drawing" && json.hasOwnProperty("drawing") && json.hasOwnProperty("totalLines"))
        || (this->waitingForLine && type == "line" && json.hasOwnProperty("command"))) {
      this->waitingForDrawing = false;
      this->waitingForLine = false;

      if (json.hasOwnProperty("drawing") && json.hasOwnProperty("totalLines")) {
        String drawing = json["drawing"];
        int totalLines = json["totalLines"];

        this->currentDrawing = drawing;
        this->totalLines = totalLines;
        this->currentLine = 1;
        this->plotter.executeWipe();
      }

      if (json.hasOwnProperty("drawing") && json.hasOwnProperty("line")) {
        String drawing = json["drawing"];
        int line = json["line"];

        if (drawing != this->currentDrawing || line != this->currentLine) {
          String msg = "Drawing=" + this->currentDrawing + ", Line=" + this->currentLine + ", Message=" + message;
          this->reportError("Incorrect drawing or line", msg);
        }
      }

      if (json.hasOwnProperty("command")) {
        String command = json["command"];

        this->plotter.computeSteps(command);
        this->hasSteps = this->plotter.getStepCount() > 0;
      }

      this->reportState();
      handled = true;
    }
  }

  if (!handled) {
    reportError("Unknown or invalid toDevice", message);
  }
}

void PlotterController::reportState() {
  Point position = this->plotter.getPosition();

  JSONVar reported;
  reported["drawing"] = this->currentDrawing;
  reported["totalLines"] = this->totalLines;
  reported["line"] = this->currentLine;
  reported["radius"] = position.getRadius();
  reported["azimuth"] = position.getAzimuth();

  JSONVar state;
  state["reported"] = reported;

  JSONVar msg;
  msg["state"] = state;

  this->publishMessage(this->updateTopic, msg);
}

void PlotterController::reportError(const String& type, const String& message) {
  JSONVar msg;
  msg["command"] = "error";
  msg["type"] = type;
  msg["message"] = message;
  msg["drawing"] = this->currentDrawing;
  msg["totalLines"] = this->totalLines;
  msg["line"] = this->currentLine;

  this->publishMessage(this->fromDeviceTopic, msg);
}
