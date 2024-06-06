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
    nextBleTime += 100;
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
