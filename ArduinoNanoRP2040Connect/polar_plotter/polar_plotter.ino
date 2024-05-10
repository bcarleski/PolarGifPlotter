#include "secrets.h"
#include "constants.h"

#include <PolarPlotterCore.h>
#include "safePrinter.h"
#include "safeStatus.h"

#include <Stepper.h>

#if USE_CLOUD > 0
#include <WiFiNINA.h>
#include "httpDrawingProducer.h"

const String drawingsHost = DRAWINGS_HOST;
const int drawingsPort = DRAWINGS_PORT;
const String drawingsFile = DRAWINGS_FILE;
const String drawingPathPrefix = DRAWING_PATH_PREFIX;

WiFiClient wifiClient;
HttpClient httpClient(wifiClient, drawingsHost, drawingsPort);
HttpDrawingProducer drawings(httpClient, drawingsFile, drawingPathPrefix);
#else
#include "knownDrawingProducer.h"
KnownDrawingProducer drawings;
#endif
#if USE_BLE > 0
BLEService bleService(BLE_SERVICE_UUID);
BLEStringCharacteristic bleCommand(BLE_COMMAND_UUID, BLEWrite | BLERead, BLE_STRING_SIZE);
SafeStatus status(bleService);
#else
SafeStatus status;
#endif

const double azimuthStepsForOneTableRotation = AZIMUTH_STEPPER_STEPS_PER_ROTATION * AZIMUTH_GEAR_RATIO;
const double radiusStepsForOneDriveGearRotation = RADIUS_STEPPER_STEPS_PER_ROTATION * RADIUS_GEAR_RATIO;
const double radiusStepsToAzimuthStep = -1.0 * radiusStepsForOneDriveGearRotation / azimuthStepsForOneTableRotation;
const double maxRadius = MAX_RADIUS;
const double marbleSizeInRadiusSteps = MARBLE_SIZE_IN_RADIUS_STEPS;
double radiusStepOffsetFromAzimuth = 0.0;
double radiusStepSize;
double azimuthStepSize;
const char* radiusStepSizeKey = "DynamicSand.RadiusStepSize";
const char* azimuthStepSizeKey = "DynamicSand.AzimuthStepSize";
SafePrinter printer;

Stepper radiusStepper(RADIUS_STEPPER_STEPS_PER_ROTATION, RADIUS_STEPPER_STEP_PIN, RADIUS_STEPPER_DIR_PIN);
Stepper azimuthStepper(AZIMUTH_STEPPER_STEPS_PER_ROTATION, AZIMUTH_STEPPER_STEP_PIN, AZIMUTH_STEPPER_DIR_PIN);
PlotterController plotter(printer, status, maxRadius, marbleSizeInRadiusSteps);

unsigned long backoffDelay = INITIAL_BACKOFF_DELAY_MILLIS;
unsigned long drawingUpdateAttempt = 0;
unsigned long waitingEndsAt = 0;
bool bleInitialized = false;
bool bleAdvertising = false;

enum LoopState {
  CYCLING,
  WAITING_WIFI,
  WAITING_STEP
};
LoopState loopState;

void setup() {
  printer.init();
  delay(2000);
  loopState = CYCLING;

  bleInitialize(true);

  status.init();

  printer.println("Starting Setup");
  status.status("START SETUP");

  radiusStepper.setSpeed(RADIUS_RPMS * RADIUS_STEP_MULTIPLIER);
  azimuthStepper.setSpeed(AZIMUTH_RPMS * AZIMUTH_STEP_MULTIPLIER);
  plotter.onStep(performStep);
  plotter.onRecalibrate(performRecalibrate);

  if (DEFAULT_DEBUG_LEVEL > 0) {
    String cmd = ".D";
    plotter.addCommand(cmd + DEFAULT_DEBUG_LEVEL);
  }

#if USE_BLE > 0
  status.setMaxRadius(maxRadius);
  status.setMarbleSizeInRadiusSteps(marbleSizeInRadiusSteps);
  status.setRadiusStepSize(0.0);
  status.setAzimuthStepSize(0.0);
  status.setCurrentDrawing("-");
  status.setCurrentCommand("-");
  status.setCurrentStep(0);
  status.setPosition("-");
  status.setState("Initializing");
  bleService.addCharacteristic(bleCommand);
  BLE.addService(bleService);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  bleCommand.setEventHandler(BLEWritten, bleCommandWritten);
  bleCommand.setValue("");
#endif

  bleAdvertise();

  String cmd = ".M";  // Start manual
  plotter.addCommand(cmd);

  printer.println("Finished Setup");
  status.status("FINISHED SETUP");
}

void loop() {
  handleSerialInput();
#if USE_BLE > 0
  BLE.poll();
#endif
  if (loopState == WAITING_STEP) {
    if (millis() < waitingEndsAt) return;
    loopState = CYCLING;
  }

  if (plotter.canCycle()) {
    plotter.performCycle();
    if (plotter.isPaused()) {
      delay(200);
    }
  } else {
#if USE_CLOUD > 0
    getCommands();
#else
    tryGetDrawingUpdate();
#endif
  }
}

const bool isBleConnected() {
#if USE_BLE > 0
  return BLE.connected();
#else
  return false;
#endif
}

void bleInitialize(bool holdOnFailure) {
  if (bleInitialized) return;

#if USE_BLE > 0
  if (Serial) Serial.println("Starting BLE");
  int bleBegin = BLE.begin();
  if (!bleBegin) {
    if (Serial) {
      Serial.print(" BLE Start failed - ");
      Serial.println(bleBegin);
    }

    while (holdOnFailure);
  } else {
    // set advertised local name and service UUID:
    BLE.setLocalName(BLE_DEVICE_NAME);
    BLE.setAdvertisedService(bleService);
  }
#endif
  bleInitialized = true;
}

void bleAdvertise() {
  if (!bleInitialized || bleAdvertising) return;

#if USE_BLE > 0
  // start advertising
  BLE.advertise();
  if (Serial) Serial.println("BLE waiting for connections");
#endif
}

void bleDestroy() {
  if (!bleInitialized) return;

#if USE_BLE > 0
  if (BLE.connected()) {
    if (Serial) Serial.println("BLE disconnecting");
    BLE.disconnect();
  }

  if (bleAdvertising) {
    if (Serial) Serial.println("BLE stopping advertise");
    BLE.stopAdvertise();
  }

  //if (Serial) Serial.println("BLE shutting down");
  //BLE.end();
#endif

  bleAdvertising = false;
  bleInitialized = false;
}

void performStep(const int radiusSteps, const int azimuthSteps, const bool fastStep) {
  // Because of how my arm is configured, an azimuth step results in a slight radius step
  // that we need to offset and account for
  int rSteps = radiusSteps;
  if (azimuthSteps != 0) {
    double radiusStepOffset = radiusStepOffsetFromAzimuth - radiusStepsToAzimuthStep * azimuthSteps;
    int steps = round(radiusStepOffset);

    if (steps != 0) {
      radiusStepOffsetFromAzimuth += radiusStepOffset - steps;
      rSteps += steps;
    }
  }

  if (!fastStep) {
    waitingEndsAt = millis() + MINIMUM_STEP_DELAY_MILLIS;
    loopState = WAITING_STEP;
  }

  radiusStepper.step(rSteps * RADIUS_STEP_MULTIPLIER);
  azimuthStepper.step(azimuthSteps * AZIMUTH_STEP_MULTIPLIER);
}

void performRecalibrate(const int maxRadiusSteps, const int fullCircleAzimuthSteps) {
  const double maxRadius = MAX_RADIUS;
  radiusStepSize = maxRadius / maxRadiusSteps;
  azimuthStepSize = (2 * PI) / fullCircleAzimuthSteps;

  plotter.calibrate(radiusStepSize, azimuthStepSize);
}

void handleSerialInput() {
  if (Serial && Serial.available()) {
    String command = Serial.readStringUntil('\n');
    plotter.addCommand(command);
  }
}

bool tryGetDrawingUpdate() {
  drawingUpdateAttempt++;
  String attempt = "ATTEMPT: ";
  printer.println("Attempting to get drawing update");
  status.status("Updating Drawing", attempt + drawingUpdateAttempt);

  if (!drawings.tryGetNewDrawing()) return false;

  String drawing = drawings.getDrawing();
  int commandCount = drawings.getCommandCount();

  printer.println("    New Drawing: " + drawing + " (" + commandCount + " commands)");
  plotter.newDrawing(drawing);

  for (int i = 0; i < commandCount; i++) {
    String command = drawings.getCommand(i);
    plotter.addCommand(command);
  }

  return true;
}

#if USE_BLE > 0
void bleCommandWritten(BLEDevice central, BLECharacteristic characteristic) {
  String command = bleCommand.value();
  if (Serial) Serial.println("Got new command from BLE: " + command);
  plotter.addCommand(command);
}

void blePeripheralConnectHandler(BLEDevice central) {
  if (!Serial) return;

  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  if (!Serial) return;

  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}
#endif

#if USE_CLOUD > 0
void getCommands() {
  if (loopState == WAITING_WIFI) {
    if (millis() < waitingEndsAt) return;
    loopState = CYCLING;
  }

  if (backoffDelay == INITIAL_BACKOFF_DELAY_MILLIS) {
    status.save();
  }
  if ((WiFi.status() == WL_CONNECTED || tryConnectWiFi()) && tryGetDrawingUpdate()) {
    status.restore();
    backoffDelay = INITIAL_BACKOFF_DELAY_MILLIS;
  } else {
    doBackoffDelay();
  }
}

void doBackoffDelay() {
  waitingEndsAt = millis() + backoffDelay;
  loopState = WAITING_WIFI;

  if (Serial) {
    Serial.print("Waiting ");
    Serial.print(backoffDelay);
    Serial.println(" ms for backoff");
  }
  backoffDelay *= 2UL;
  if (backoffDelay > MAX_BACKOFF_DELAY_MILLIS) {
    backoffDelay = MAX_BACKOFF_DELAY_MILLIS;
  }
}

bool tryConnectWiFi() {
  printer.println("Attempting to connect to wifi: ");
  //bleDestroy();

  for (int i = 0; i < NETWORK_PROFILE_COUNT; i++) {
    NetworkProfile profile = networkProfiles[i];
    String ssid = profile.getSsid();
    status.status("WIFI Connecting", ssid);
    if (WiFi.begin(ssid.c_str(), profile.getPass().c_str()) == WL_CONNECTED) {
      printer.println("WIFI Connected to " + ssid);
      return true;
    }
  }

  printer.println("WIFI connect FAILED");
  return false;
}
#endif