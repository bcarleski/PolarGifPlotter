#include "constants.h"

#include "safeStatus.h"

SafePrinter printer;

#if USE_BLE > 0
#define POLL_MILLIS_INTERVAL 100

BLEService bleService(BLE_SERVICE_UUID);
BLEStringCharacteristic bleCommand(BLE_COMMAND_UUID, BLEWrite | BLERead, BLE_STRING_SIZE);
SafeStatus status(printer, bleService);
unsigned long nextPollMillis = 0;

void bleCommandWritten(BLEDevice central, BLECharacteristic characteristic) {
  String command = bleCommand.value();
  printer.println("BLE input: " + command);
  plotter.addCommand(command);
}

void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  printer.print("Connected event, central: ");
  printer.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  printer.print("Disconnected event, central: ");
  printer.println(central.address());
}

#else
SafeStatus status(printer);
#endif

void bleInit() {
#if USE_BLE > 0
  printer.println("Starting BLE");
  int bleBegin = BLE.begin();
  if (!bleBegin) {
    printer.print(" BLE Start failed - ");
    printer.println(bleBegin);

    while (true);
  } else {
    // set advertised local name and service UUID:
    BLE.setLocalName(BLE_DEVICE_NAME);
    BLE.setAdvertisedService(bleService);
  }
#endif
}

void bleAdvertise() {
#if USE_BLE > 0
  status.setMaxRadius(maxRadius);
  status.setMarbleSizeInRadiusSteps(marbleSizeInRadiusSteps);
  status.setRadiusStepSize(0.0);
  status.setAzimuthStepSize(0.0);
  status.setCurrentDrawing("-");
  status.setCurrentCommand("-");
  status.setCurrentStep(0);
  status.setPosition(0, 0);
  status.setState("Initializing");
  bleService.addCharacteristic(bleCommand);
  BLE.addService(bleService);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  bleCommand.setEventHandler(BLEWritten, bleCommandWritten);
  bleCommand.setValue("");

  // start advertising
  BLE.advertise();
  printer.println("BLE waiting for connections");
#endif
}

void bleLoop(unsigned long currentMillis) {
#if USE_BLE > 0
  if (nextPollMillis <= currentMillis) {
    BLE.poll();
    nextPollMillis = millis() + POLL_MILLIS_INTERVAL;
  }
#endif
}