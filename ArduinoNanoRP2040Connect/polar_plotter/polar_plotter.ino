#include "secrets.h"
#include "constants.h"

#include <PolarPlotterCore.h>
#include <Stepper.h>

#if USE_CLOUD > 0
#include <ArduinoBearSSL.h>
#include <ArduinoECCX08.h>
#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>  // change to #include <WiFi101.h> for MKR1000

const String deviceName = IOT_DEVICE_NAME;
const String deviceShadowName = IOT_DEVICE_SHADOW_NAME;
const char broker[] = SECRET_BROKER;
const char* certificate = SECRET_CERTIFICATE;

WiFiClient wifiClient;                // Used for the TCP socket connection
BearSSLClient sslClient(wifiClient);  // Used for SSL/TLS connection, integrates with ECC508
MqttClient mqttClient(sslClient);
#endif
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
CondOut condOut(lcd, USE_LCD != 0);
Stepper radiusStepper(RADIUS_STEPPER_STEPS_PER_ROTATION, RADIUS_STEPPER_STEP_PIN, RADIUS_STEPPER_DIR_PIN);
Stepper azimuthStepper(AZIMUTH_STEPPER_STEPS_PER_ROTATION, AZIMUTH_STEPPER_STEP_PIN, AZIMUTH_STEPPER_DIR_PIN);
PolarPlotter plotter(condOut, MAX_RADIUS, RADIUS_STEP_SIZE, AZIMUTH_STEP_SIZE, MARBLE_SIZE_IN_RADIUS_STEPS);
PlotterController controller(plotter, IOT_DEVICE_NAME, IOT_DEVICE_SHADOW_NAME);
unsigned long lastCloudCheck = 0;

void setup() {
  condOut.init();
  condOut.println("Starting Setup");
  condOut.lcdPrint("START SETUP", "");

  controller.onMessage(publishMessage);
  controller.onMqttPoll(pollMqtt);
  radiusStepper.setSpeed(RADIUS_RPMS);
  azimuthStepper.setSpeed(AZIMUTH_RPMS);
  plotter.onStep(performStep);

#if USE_CLOUD > 0
  if (!ECCX08.begin()) {
    condOut.lcdPrint("No ECCX08");
    condOut.println("No ECCX08 present!");
    while (true)
      ;
  }

  // Set a callback to get the current time
  // used to validate the servers certificate
  ArduinoBearSSL.onGetTime(getTime);

  // Set the ECCX08 slot to use for the private key
  // and the accompanying public certificate for it
  sslClient.setEccSlot(0, certificate);

  // Optional, set the client id used for MQTT,
  // each device that is connected to the broker
  // must have a unique client id. The MQTTClient will generate
  // a client id for you based on the millis() value if not set
  //
  mqttClient.setId(deviceName);

  // Set the message callback, this function is
  // called when the MQTTClient receives a message
  mqttClient.onMessage(onMessageReceived);
#endif

  condOut.println("Finished Setup");
  condOut.lcdPrint("FINISHED SETUP", "");
}

void loop() {
  controller.performCycle();
}

bool pollMqtt(String topics[]) {
  unsigned long curTime = millis();
  // Look for updates from the cloud only every half second
  if ((curTime - lastCloudCheck) > 500) {
#if USE_CLOUD > 0
    if (WiFi.status() == WL_CONNECTED || tryConnectWiFi()) {
      if (mqttClient.connected() || tryConnectMQTT(topics)) {
        // poll for new MQTT messages and send keep alives
        mqttClient.poll();

        lastCloudCheck = millis();
        return true;
      }
    }

    lastCloudCheck = millis();
#else
    lastCloudCheck = millis();
    return true;
#endif
  }

  return false;
}

void performStep(const Step& step) {
  Serial.println("Entered step");
  // String cmd = "Stepping RADIUS=";
  // int radiusStep = step.getRadiusStep();
  // int azimuthStep = step.getAzimuthStep();
  // condOut.println(cmd + radiusStep + ", AZIMUTH=" + azimuthStep);
//  radiusStepper.step(radiusSteps);
//  azimuthStepper.step(azimuthSteps);
}

void publishMessage(const String& topic, const JSONVar& payload) {
  const String message = JSON.stringify(payload);
  condOut.print("Publishing message: TOPIC=" + topic);
  condOut.println(", MESSAGE=" + message);

#if USE_CLOUD > 0
  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic);
  mqttClient.print(message);
  mqttClient.endMessage();
#endif

  unsigned long curTime = millis();
  String time = "";
  condOut.lcdPrint("Sent message at", time + curTime);
}

#if USE_CLOUD > 0
unsigned long getTime() {
  // get the current time from the WiFi module
  return WiFi.getTime();
}

bool tryConnectWiFi() {
  condOut.print("Attempting to connect to wifi: ");

  for (int i = 0; i < NETWORK_PROFILE_COUNT; i++) {
    NetworkProfile profile = networkProfiles[i];
    String ssid = profile.getSsid();
    if (WiFi.begin(ssid.c_str(), profile.getPass().c_str()) == WL_CONNECTED) {
      condOut.println("Connected to " + ssid);
      return true;
    }
  }

  condOut.println("FAILED");
  condOut.lcdPrint("No WiFi found", "");
  return false;
}

bool tryConnectMQTT(String topics[]) {
  condOut.print("Attempting to MQTT broker: ");
  condOut.print(broker);
  condOut.print(" - ");

  if (mqttClient.connect(broker, 8883)) {
    condOut.println("Connected");

    // subscribe to a topic
    for (int i = 0; i < TOPIC_SUBSCRIPTION_COUNT; i++) {
      String topic = topics[i];
      condOut.println("Subscribing to " + topic);

      mqttClient.subscribe(topic);
    }

    return true;
  }

  condOut.println("FAILED");
  condOut.lcdPrint("No MQTT", "");
  return false;
}

void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  const String topic = mqttClient.messageTopic();

  // use the Stream interface to print the contents
  char bytes[messageSize]{};
  mqttClient.readBytes(bytes, messageSize);

  const String message = bytes;
  const String blank = "";
  condOut.println("Received message: TOPIC=" + topic + ", MESSAGE=" + message);
  condOut.lcdPrint("Received message", blank + messageSize);
  controller.messageReceived(topic, message);
}
#endif