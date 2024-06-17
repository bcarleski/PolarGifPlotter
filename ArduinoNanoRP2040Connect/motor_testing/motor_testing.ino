#include <TMCStepper.h>
#define STP_PIN D15
#define DIR_PIN D14
#define SERIAL_PORT Serial1
#define R_SENSE 0.11f

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, 0);

unsigned long currentStepTime;
unsigned long nextOutputTime;
unsigned long currentBgTime;
unsigned long nextBgLoopTime = 0;
unsigned long nextModeSwitch = 0;
unsigned long mode = -1;
long motorPosition = 0;
bool lastReverse = false;

void setup() {
  rp2040.enableDoubleResetBootloader();
  rp2040.idleOtherCore();

  Serial.begin(115200);
  SERIAL_PORT.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  delay(2000);
  if (Serial) Serial.println("Starting");

  delay(2000);

  if (Serial) Serial.println("Setting up motor");
  setupMotor();
  if (Serial) Serial.println("Motor setup");
  //setupBackground();

  delay(2000);

  rp2040.restartCore1();

  if (Serial) Serial.println("Finished Setup");
}

// void setup1() {
//   sleep_ms(1000);
//   setupMotor();
// }

void loop() {
  currentStepTime = millis();
  runMotor(currentStepTime);

//  checkBackground(currentStepTime);
  if (nextOutputTime <= currentStepTime) {
    Serial.print("Main loop - ");
//    outputTime(currentStepTime);
    Serial.print(motorPosition);
    Serial.println();
    nextOutputTime = currentStepTime + 2000;
  }
}

// void loop1() {
//   currentBgTime = millis();
//   runMotor(currentBgTime);
// }

void setupMotor() {
  driver.begin();
  driver.reset();
  driver.toff(5);
  driver.microsteps(0);
  driver.intpol(true);
  sleep_ms(2000);
  pinMode(STP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  sleep_ms(2000);
  digitalWrite(STP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
//  digitalWrite(LED_BUILTIN, LOW);
  sleep_ms(2000);
}

void runMotor(unsigned long currentTime) {
  if (nextBgLoopTime <= currentTime) {
    nextBgLoopTime = currentTime + 20;

    if (nextModeSwitch <= currentTime) {
      mode++;
      nextModeSwitch = currentTime + 5000;
    }

    bool reverse = false;
    bool step = false;
    int mod = mode % 4;
    switch (mod) {
      case 0: step = true; reverse = true; break;
      case 1: step = false; reverse = true; break;
      case 2: step = true; reverse = false; break;
      case 3: step = false; reverse = true; break;
    }

    if (reverse != lastReverse) {
      int dir = reverse ? HIGH : LOW;
      //digitalWrite(LED_BUILTIN, dir);
      digitalWrite(DIR_PIN, dir);
      lastReverse = reverse;
      delayMicroseconds(20);
    }
    if (step) {
      motorPosition += (reverse ? -1 : 1);
      digitalWrite(STP_PIN, HIGH);
      delayMicroseconds(20);
      digitalWrite(STP_PIN, LOW);
    }
  }
}
