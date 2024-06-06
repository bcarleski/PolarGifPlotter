#include "background.h"
#include "bt.h"

unsigned long currentStepTime;
unsigned long nextOutputTime;
unsigned long nextBleTime;
unsigned long currentBgTime;
unsigned long nextBgLoopTime = 0;

void setup() {
  // Add a delay of 10s in case we break anything, we have a change to republish
  delay(10000);

  rp2040.idleOtherCore();

  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  if (Serial) Serial.println("Starting");

  delay(2000);

  setupBluetooth();
  setupBackground();

  delay(2000);

  startBluetooth();

  rp2040.restartCore1();

  if (Serial) Serial.println("Finished Setup");
}

void setup1() {
  // Add a delay of 10s in case we break anything, we have a change to republish
  delay(10000);
}

void loop() {
  currentStepTime = millis();

  checkBackground(currentStepTime);
  if (nextOutputTime <= currentStepTime) {
    Serial.print("Main loop - ");
    outputTime(currentStepTime);
    Serial.println();
    nextOutputTime += 5000;
  }

  if (nextBleTime <= currentStepTime) {
    bluetoothLoop();
    nextBleTime += 500;
  }
}

void loop1() {
  currentBgTime = millis();

  if (nextBgLoopTime <= currentBgTime) {
    nextBgLoopTime += 100;
    backgroundLoop();
  } else {
    sleep_ms(40);
  }
}

void setupBluetooth() {
  BTstack.setBLEDeviceConnectedCallback(deviceConnectedCallback);
  BTstack.setBLEDeviceDisconnectedCallback(deviceDisconnectedCallback);
  BTstack.setGATTCharacteristicRead(gattReadCallback);
  BTstack.setGATTCharacteristicWrite(gattWriteCallback);

  BTstack.addGATTService(new UUID("B8E06067-62AD-41BA-9231-206AE80AB551"));
  BTstack.addGATTCharacteristic(new UUID("f897177b-aee8-4767-8ecc-cc694fd5fcef"), ATT_PROPERTY_READ, "This is a String!");
  BTstack.addGATTCharacteristicDynamic(new UUID("f897177b-aee8-4767-8ecc-cc694fd5fce0"), ATT_PROPERTY_READ | ATT_PROPERTY_WRITE | ATT_PROPERTY_NOTIFY, 0);
}

void startBluetooth() {
  Serial.println("Calling Bluetooth Setup");
  BTstack.setup();

  Serial.println("Starting Bluetooth Advertising");
  BTstack.startAdvertising();
}

void bluetoothLoop() {
  BTstack.loop();
}
