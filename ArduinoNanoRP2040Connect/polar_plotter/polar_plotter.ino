#include "secrets.h"
#include "constants.h"

#include <PolarPlotterCore.h>
#include <Stepper.h>

#if USE_CLOUD > 0
#include <WiFiNINA.h>
#include "drawingClient.h"

const String drawingsHost = DRAWINGS_HOST;
const int drawingsPort = DRAWINGS_PORT;
const String drawingsFile = DRAWINGS_FILE;
const String drawingPathPrefix = DRAWING_PATH_PREFIX;

WiFiClient wifiClient;
HttpClient httpClient(wifiClient, drawingsHost, drawingsPort);
DrawingClient drawings(httpClient, drawingsFile, drawingPathPrefix);
#endif

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
CondOut condOut(lcd, USE_LCD != 0);
Stepper radiusStepper(RADIUS_STEPPER_STEPS_PER_ROTATION, RADIUS_STEPPER_STEP_PIN, RADIUS_STEPPER_DIR_PIN);
Stepper azimuthStepper(AZIMUTH_STEPPER_STEPS_PER_ROTATION, AZIMUTH_STEPPER_STEP_PIN, AZIMUTH_STEPPER_DIR_PIN);
PolarPlotter plotter(condOut, MAX_RADIUS, RADIUS_STEP_SIZE, AZIMUTH_STEP_SIZE, MARBLE_SIZE_IN_RADIUS_STEPS);
PlotterController controller(condOut, plotter);

unsigned long backoffDelay = INITIAL_BACKOFF_DELAY_MILLIS;
unsigned long drawingUpdateAttempt = 0;
unsigned long lastStepTimeMillis = 0;

void setup() {
  condOut.init();
  delay(2000);

  condOut.println("Starting Setup");
  condOut.lcdPrint("START SETUP", "");

  radiusStepper.setSpeed(RADIUS_RPMS * RADIUS_STEP_MULTIPLIER);
  azimuthStepper.setSpeed(AZIMUTH_RPMS * AZIMUTH_STEP_MULTIPLIER);
  plotter.onStep(performStep);

  condOut.println("Finished Setup");
  condOut.lcdPrint("FINISHED SETUP", "");
}

void loop() {
  handleSerialInput();
  controller.performCycle();

#if USE_CLOUD > 0
  if (controller.needsCommands()) getCommands();
#endif
}

void performStep(const int radiusSteps, const int azimuthSteps, const bool fastStep) {
  unsigned long curTime = millis();
  long delayMillis = MINIMUM_STEP_DELAY_MILLIS - (curTime - lastStepTimeMillis);
  if (!fastStep && delayMillis > 0) {
    unsigned long timeToDelay = (unsigned long)delayMillis;
    delay(timeToDelay);
  }

  lastStepTimeMillis = millis();
  radiusStepper.step(radiusSteps * RADIUS_STEP_MULTIPLIER);
  azimuthStepper.step(azimuthSteps * AZIMUTH_STEP_MULTIPLIER);
}

void handleSerialInput() {
  if (Serial && Serial.available()) {
    String command = Serial.readStringUntil('\n');

    Serial.println("Got: " + command);
    if (command.startsWith("d") || command.startsWith("D")) {
      unsigned int debugLevel = (unsigned int)command.substring(1).toInt();
      plotter.setDebug(debugLevel);
    } else if (command.startsWith("h") || command.startsWith("H")) {
      Serial.println(PolarPlotter::getHelpMessage());
    } else if (command == "w" || command == "W") {
      condOut.lcdPrint("WIPING", "");
      condOut.lcdSave();
      plotter.executeWipe();
    } else {
      controller.addCommand(command);
    }
  }
}

#if USE_CLOUD > 0
unsigned long getTime() {
  // get the current time from the WiFi module
  unsigned long wifiTime = WiFi.getTime();

  while (wifiTime == 0) {
    delay(500);
    wifiTime = WiFi.getTime();
  }

  Serial.print("Got wifi time as ");
  Serial.println(wifiTime);
}

void getCommands() {
    if ((WiFi.status() == WL_CONNECTED || tryConnectWiFi()) && tryGetDrawingUpdate()) {
      condOut.lcdPrintSaved();
      backoffDelay = INITIAL_BACKOFF_DELAY_MILLIS;
    } else {
      doBackoffDelay();
    }
}

void doBackoffDelay() {
  Serial.print("Waiting ");
  Serial.print(backoffDelay);
  Serial.println(" ms for backoff");
  delay(backoffDelay);
  backoffDelay *= 2UL;
  if (backoffDelay > MAX_BACKOFF_DELAY_MILLIS) {
    backoffDelay = MAX_BACKOFF_DELAY_MILLIS;
  }
}

bool tryConnectWiFi() {
  condOut.print("Attempting to connect to wifi: ");

  for (int i = 0; i < NETWORK_PROFILE_COUNT; i++) {
    NetworkProfile profile = networkProfiles[i];
    String ssid = profile.getSsid();
    condOut.lcdPrint("WIFI Connecting", ssid);
    if (WiFi.begin(ssid.c_str(), profile.getPass().c_str()) == WL_CONNECTED) {
      condOut.println("Connected to " + ssid);
      return true;
    }
  }

  condOut.println("FAILED");
  return false;
}

bool tryGetDrawingUpdate() {
  drawingUpdateAttempt++;
  String attempt = "ATTEMPT: ";
  condOut.println("Attempting to get drawing update");
  condOut.lcdPrint("Updating Drawing", attempt + drawingUpdateAttempt);

  if (!drawings.tryGetNewDrawing()) return false;

  String drawing = drawings.getDrawing();
  int commandCount = drawings.getCommandCount();

  condOut.println("    New Drawing: " + drawing + " (" + commandCount + " commands)");
  controller.newDrawing(drawing);

  for (int i = 0; i < commandCount; i++) {
    String command = drawings.getCommand(i);
    controller.addCommand(command);
  }

  return true;
}
#endif