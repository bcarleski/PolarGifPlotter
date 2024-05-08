#include "secrets.h"
#include "constants.h"

#include <kvstore_global_api.h>
#include <mbed_error.h>
#include <PolarPlotterCore.h>
#include "safePrinter.h"
#include "safeStatus.h"

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


#if USE_BLE > 0
  int bleBegin = BLE.begin();
  if (!bleBegin) {
    if (Serial) {
      Serial.print("Starting Bluetooth® Low Energy failed!");
      Serial.println(bleBegin);
    }
    while (!bleBegin)
      ;
  }
  // set advertised local name and service UUID:
  BLE.setLocalName(BLE_DEVICE_NAME);
  BLE.setAdvertisedService(bleService);
#endif

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

  bool radiusLoaded = loadSavedDouble(radiusStepSizeKey, &radiusStepSize);
  bool azimuthLoaded = loadSavedDouble(azimuthStepSizeKey, &azimuthStepSize);

  if (radiusLoaded && azimuthLoaded) {
    plotter.calibrate(RADIUS_STEP_SIZE, AZIMUTH_STEP_SIZE);
  } else {
    String cmd = ".C";  // Start calibration
    plotter.addCommand(cmd);
  }

#if USE_BLE > 0
  bleService.addCharacteristic(bleCommand);
  BLE.addService(bleService);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  status.setMaxRadius(maxRadius);
  status.setMarbleSizeInRadiusSteps(marbleSizeInRadiusSteps);
  bleCommand.setEventHandler(BLEWritten, bleCommandWritten);
  bleCommand.setValue("");
  // start advertising
  BLE.advertise();
  if (Serial) Serial.println("BLE waiting for connections");
#endif

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
#if USE_CLOUD > 0
  } else {
    getCommands();
#endif
  }
}

bool saveDouble(const char* stringKey, double value) {
  String val(value, 6);
  int result = kv_set(stringKey,  val.c_str(), val.length() + 1, 0);
  return result == MBED_SUCCESS;
}

bool loadSavedDouble(const char* stringKey, double* value) {
  kv_info_t infoBuffer;
  int result = kv_get_info(stringKey, &infoBuffer);

  if (result = MBED_SUCCESS) {
    char *readBuffer = new char[infoBuffer.size];
    size_t actualSize;
    memset(readBuffer, 0, infoBuffer.size);
    result = kv_get(stringKey, readBuffer, infoBuffer.size, &actualSize);

    if (result == MBED_SUCCESS) {
      String str(readBuffer, actualSize);
      *value = str.toDouble();
      return true;
    }

    delete[] readBuffer;
  }

  return false;
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
  saveDouble(radiusStepSizeKey, radiusStepSize);
  saveDouble(azimuthStepSizeKey, azimuthStepSize);
}

void handleSerialInput() {
  if (Serial && Serial.available()) {
    String command = Serial.readStringUntil('\n');
    plotter.addCommand(command);
  }
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
  printer.print("Attempting to connect to wifi: ");

  for (int i = 0; i < NETWORK_PROFILE_COUNT; i++) {
    NetworkProfile profile = networkProfiles[i];
    String ssid = profile.getSsid();
    status.status("WIFI Connecting", ssid);
    if (WiFi.begin(ssid.c_str(), profile.getPass().c_str()) == WL_CONNECTED) {
      printer.println("Connected to " + ssid);
      return true;
    }
  }

  printer.println("FAILED");
  return false;
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
#endif