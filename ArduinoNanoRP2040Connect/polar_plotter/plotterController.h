#include "polarPlotter.h"
#include <Arduino_JSON.h>

#define TOPIC_SUBSCRIPTION_COUNT 3

class PlotterController {
  private:
    PolarPlotter plotter;
    const String getTopic;
    const String getAcceptedTopic;
    const String getRejectedTopic;
    const String updateTopic;
    const String toDeviceTopic;
    const String fromDeviceTopic;
    String topics[TOPIC_SUBSCRIPTION_COUNT];
    void (*publishMessage)(const String&, const JSONVar&);
    bool (*pollMqtt)(String[]);
    bool initialized;
    bool waitingForDeviceShadow;
    bool waitingForDrawing;
    bool waitingForLine;
    bool hasSteps;
    String currentDrawing;
    int currentLine;
    int totalLines;

    void requestDrawing();
    void requestLine();
    void getAcceptedMessage(const String&);
    void getRejectedMessage(const String&);
    void toDeviceMessage(const String&);
    void reportState();
    void reportError(const String&, const String&);

  public:
    PlotterController(PolarPlotter&, const String&, const String&);
    void onMessage(void publishMessage(const String&, const JSONVar&));
    void onMqttPoll(bool pollMqtt(String[]));
    void performCycle();
    void messageReceived(const String&, const String&);
};
