#include "secrets.h"
#include "constants.h"
#include <PolarPlotterCore.h>
#include "backgroundMotorCoordinator.h"
#include "bleFunctions.h"

SafePrinter printer;
SafeStatus status(printer);

#if USE_CLOUD > 0
#include "httpDrawingProducer.h"
HttpDrawingProducer drawings(printer, httpClient, drawingsFile, drawingPathPrefix);
#else
#include "drawingProducer.h"
KnownDrawingProducer drawings(printer);
#endif

PolarBle bluetooth(printer, status);
BackgroundMotorCoordinator coordinator(printer, RADIUS_ADDRESS, RADIUS_STEP_PIN, RADIUS_DIR_PIN,
                                       AZIMUTH_ADDRESS, AZIMUTH_STEP_PIN, AZIMUTH_DIR_PIN);
PlotterController plotter(printer, status, maxRadius, marbleSizeInRadiusSteps, &coordinator);

unsigned long backoffDelay = INITIAL_BACKOFF_DELAY_MILLIS;
unsigned long drawingUpdateAttempt = 0;
unsigned long nextWifiCheckTime = 0;
unsigned long currentMillis = 0;
unsigned long nextSerialCheckTime = 0;
bool bleInitialized = false;
bool bleAdvertising = false;



// ####################################################################################################
// #                                              SETUP                                               #
// ####################################################################################################
void setup() {
  rp2040.enableDoubleResetBootloader();
  rp2040.idleOtherCore();

  printer.init();
  coordinator.init();
  plotter.onRecalibrate(performRecalibrate);

  delay(2000);
  bluetooth.init();

  printer.println("Starting Setup");
  status.status("START SETUP");

  String cmd = ".M";  // Start manual
  advertiseBluetooth(&bluetooth, handleCommand);
  plotter.addCommand(cmd);
  rp2040.restartCore1();

  printer.println("Finished Setup");
  status.status("FINISHED SETUP");
}

void setup1() {
  sleep_ms(2000);
  coordinator.backgroundInit();

  sleep_ms(2000);
  coordinator.backgroundBegin();
  sleep_ms(2000);
}




// ####################################################################################################
// #                                               LOOP                                               #
// ####################################################################################################
void loop() {
  currentMillis = millis();

  handleSerialInput();
  bluetooth.loop(currentMillis);

  if (plotter.canCycle()) {
    plotter.performCycle();
  } else {
    getCommands();
  }
}

void loop1() {
  coordinator.backgroundLoop();
}




// ####################################################################################################
// #                                             HELPERS                                              #
// ####################################################################################################
void performRecalibrate(const int maxRadiusSteps, const int fullCircleAzimuthSteps) {
  const double maxRadius = MAX_RADIUS;
  double radiusStepSize = maxRadius / maxRadiusSteps;
  double azimuthStepSize = (2 * PI) / fullCircleAzimuthSteps;

  plotter.calibrate(radiusStepSize, azimuthStepSize);
}

void handleSerialInput() {
#if USE_SERIAL > 0
  if (nextSerialCheckTime > currentMillis) return;

  nextSerialCheckTime += INPUT_CHECK_WAIT_TIME;
  if (Serial && Serial.available()) {
    String command = Serial.readStringUntil('\n');
    printer.print("Serial input: ");
    printer.println(command);
    handleCommand(command);
  }
#endif
}

void handleCommand(String& command) {
  if (command.equalsIgnoreCase(".RESTART")) {
    printer.println("RESTARTING");
    delay(1000);
    rp2040.reboot();
  }
  plotter.addCommand(command);
}

void getCommands() {
#if USE_CLOUD > 0
  if (currentMillis < nextWifiCheckTime) return;

  if (backoffDelay == INITIAL_BACKOFF_DELAY_MILLIS) {
    status.save();
  }
  if ((WiFi.status() == WL_CONNECTED || tryConnectWiFi()) && drawings.tryGetNewDrawing(status, plotter)) {
    status.restore();
    backoffDelay = INITIAL_BACKOFF_DELAY_MILLIS;
  } else {
    doBackoffDelay();
  }
#else
  drawings.tryGetNewDrawing(status, plotter);
#endif
}

#if USE_CLOUD > 0
void doBackoffDelay() {
  nextWifiCheckTime = currentMillis + backoffDelay;

  printer.print("Waiting ");
  printer.print(backoffDelay);
  printer.println(" ms for backoff");
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