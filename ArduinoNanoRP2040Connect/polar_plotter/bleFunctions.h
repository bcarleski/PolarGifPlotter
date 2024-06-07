#include "constants.h"
#include "safeStatus.h"
#define POLL_MILLIS_INTERVAL 10

class PolarBle {
private:
  SafePrinter& printer;
  SafeStatus& status;
  unsigned long nextPollMillis;

public:
  PolarBle(SafePrinter& printer, SafeStatus& status)
    : printer(printer),
      status(status)
#if USE_BLE > 0
      , bleStatus(BleSafeStatus()),
      bleService(BLEService(BLE_SERVICE_UUID)),
      bleCommand(BLEStringCharacteristic(BLE_COMMAND_UUID, BLEWrite | BLERead, BLE_STRING_SIZE))
#endif
  {}

#if USE_BLE > 0
  BLEService bleService;
  BLEStringCharacteristic bleCommand;
  BleSafeStatus bleStatus;
  void (*bleCommandHandler)(String& command);

  void bleCommandWritten(BLEDevice central, BLECharacteristic characteristic) {
    String command = bleCommand.value();
    printer.println("BLE input: " + command);
    if (bleCommandHandler) bleCommandHandler(command);
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
#endif

  void init() {
#if USE_BLE > 0
    printer.println("Starting BLE");
    int bleBegin = BLE.begin();
    if (!bleBegin) {
      printer.print(" BLE Start failed - ");
      printer.println(bleBegin);

      while (true);
    }

    // set advertised local name and service UUID:
    BLE.setLocalName(BLE_DEVICE_NAME);
    BLE.setAdvertisedService(bleService);

    bleStatus.init(bleService);
    status.initBle(&bleStatus);
#endif
  }

  void advertisePreHandlers() {
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
#endif
  }

  void advertisePostHandlers() {
#if USE_BLE > 0
    bleCommand.setValue("");

    // start advertising
    BLE.advertise();
    printer.println("BLE waiting for connections");
#endif
  }

  void loop(unsigned long currentMillis) {
#if USE_BLE > 0
    if (nextPollMillis <= currentMillis) {
      BLE.poll();
      nextPollMillis = millis() + POLL_MILLIS_INTERVAL;
    }
#endif
  }
};

#if USE_BLE > 0
PolarBle* defaultPolarBle;
void bleCommandWritten(BLEDevice central, BLECharacteristic characteristic) { if (defaultPolarBle) defaultPolarBle->bleCommandWritten(central, characteristic); }
void blePeripheralConnectHandler(BLEDevice central) { if (defaultPolarBle) defaultPolarBle->blePeripheralConnectHandler(central); }
void blePeripheralDisconnectHandler(BLEDevice central) { if (defaultPolarBle) defaultPolarBle->blePeripheralConnectHandler(central); }
#endif

void advertiseBluetooth(PolarBle* polarBle, void handleCommand(String& command)) {
#if USE_BLE > 0
  defaultPolarBle = polarBle;
  polarBle->bleCommandHandler = handleCommand;
  polarBle->advertisePreHandlers();
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  polarBle->bleCommand.setEventHandler(BLEWritten, bleCommandWritten);
  polarBle->advertisePostHandlers();
#endif
}
