#define BLE_DEVICE_NAME "Dynamic_Sand_Arduino"
#define BLE_SERVICE_UUID "45aa5c8f-c47e-42f6-af4a-66544b8aff17"
#define BLE_STATUS_UUID "52eb19a4-6421-4910-a8ca-7ff75ef2f56b"
#define BLE_COMMAND_UUID "66af95bc-3dd1-4343-b4b5-ad328b33fda7"
#define BLE_MAX_RADIUS_UUID "fb65af7d-494f-4a45-8872-6e6ffbf0703c"
#define BLE_RADIUS_STEP_SIZE_UUID "fd18cf65-85d7-4730-ad77-4cc3fabaab99"
#define BLE_AZIMUTH_STEP_SIZE_UUID "4dc9c338-0004-4c05-bd26-bb13e55c3bb9"
#define BLE_MARBLE_SIZE_UUID "60af168a-b702-4d0b-8c1b-f35c7a436781"
#define BLE_DRAWING_UUID "fa95bee6-46f9-4898-913a-0575019d3d33"
#define BLE_STEP_UUID "54a63a69-90ce-4b14-a103-46152bb1da68"
#define BLE_RADIUS_UUID "7fcd311a-fafa-47ee-80b8-618616697a59"
#define BLE_AZIMUTH_UUID "eb654acc-3430-45e3-8dc9-22c9fe982518"
#define BLE_STATE_UUID "ec314ea1-7426-47fb-825c-8fbd8b02f7fe"
#define BLE_STRING_SIZE 512
#define SERIAL_POLL_INTERVAL 2000000UL
#define POSITION_UPDATE_INTERVAL 2000
#define STEP_UPDATE_INTERVAL 2000
// #define STEPPER_MS1_PIN 14
// #define STEPPER_MS2_PIN 15
// #define RADIUS_STEPPER_STEP_PIN 16
// #define RADIUS_STEPPER_DIR_PIN 17
// #define AZIMUTH_STEPPER_STEP_PIN 18
// #define AZIMUTH_STEPPER_DIR_PIN 19
// #define MS1_PIN 14
// #define MS2_PIN 15
#define STEP_PIN 16
#define DIR_PIN 17
#define FAST_OFFSET 110  // LOW, LOW, 110
#define SLOW_OFFSET 550  // LOW, HIGH, 550
#define SERIAL_PORT Serial1 // HardwareSerial port pins 0 & 1
#define DRIVER_ADDRESS 0b00
#define R_SENSE 0.11f

#include <SpeedyStepper.h>
#include <TMCStepper.h>

const uint8_t REPLY_DELAY = 4;
const long SERIAL_BAUD_RATE = 115200;

unsigned long currentStepTime = 0;
unsigned long nextStepUpdateTime = 0;
unsigned long nextPositionUpdateTime = 0;
unsigned long nextSerialPollTime = 0;
unsigned long nextStepTime = 0;
unsigned long nextStepTimeOffset = 4000;
unsigned long currentStep = 0;
long stepLimit = 2000;
long stepNumber = 0;
long gapWaitTime = 1000;
bool adjustingAzimuth = false;
bool running = false;
bool shaft = false;
bool direction = false;

TMC2209Stepper driver(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS);

//SpeedyStepper stepper;

void setup() {
  // pinMode(MS1_PIN, OUTPUT);
  // pinMode(MS2_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  SERIAL_PORT.begin(115200);
  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  if (Serial) Serial.println("Starting");

  delay(2000);

  // digitalWrite(MS1_PIN, LOW);
  // digitalWrite(MS2_PIN, LOW);
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  driver.begin();
  driver.reset();
  driver.toff(5);
  // driver.rms_current(2000, 0.1);
  // driver.pwm_autoscale(false);
  // driver.en_spreadCycle(false);
  driver.microsteps(2);
  driver.intpol(true);

  delay(2000);

  if (Serial) Serial.println("Finished Setup");

  readSettings();
  delay(500);
}

void loop() {
  // uint16_t msread=driver.microsteps();
  // Serial.print(F("Read microsteps via UART to test UART receive : "));    Serial.println(msread); 

  // Serial.println(F("Move steps forward"));
  // // driver.rms_current(600);
  // if (direction) {
  //   digitalWrite(DIR_PIN, HIGH);
  //   delayMicroseconds(20);
  // }
  // int i;
  // for (i = 0; i < stepLimit; i++) {
  //   digitalWrite(STEP_PIN, HIGH);
  //   delayMicroseconds(4);
  //   digitalWrite(STEP_PIN, LOW);
  //   delayMicroseconds(nextStepTimeOffset);
  // }
  // // stepper.moveToPositionInSteps(6400);
  // // digitalWrite(DIR_PIN, LOW);

  // if (direction) digitalWrite(DIR_PIN, LOW);

  // Serial.println(F("Wait and switch direction"));
  // // driver.rms_current(10); 
  // delay(gapWaitTime);

//  Serial.println(F("Move back to 0 position at 300ma"));
//  driver.rms_current(300); 
  // stepper.moveToPositionInSteps(0);

  // for (i = 0; i < 3200; i++) {
  //   digitalWrite(STEP_PIN, HIGH);
  //   delayMicroseconds(4);
  //   digitalWrite(STEP_PIN, LOW);
  //   delayMicroseconds(600);
  // }
  // digitalWrite(DIR_PIN, LOW);

//  Serial.println(F("Automove motor for 2 seconds, then switch direction"));
  // delay(1000);

  //MOVE MOTOR VIA UART AND CHANGE DIRECTION VIA SOFTWARE, IT RUNS AS LONG AS YOU LET IT... PROBABLY ONLY USEFUL WITH ENCODER. THE VALUE SETS ONLY THE SPEED.
  // driver.VACTUAL(16000); //SET SPEED OF MOTOR
  // delay(2000); // MOTOR MOVES 2 SEC THEN STOPS
  // driver.VACTUAL(0); //STOP MOTOR BY SETTING SPEED TO 0
  // shaft = !shaft; // REVERSE DIRECTION
  // driver.shaft(shaft); // SET DIRECTION
  currentStepTime = micros();

  if (running) {
    if (currentStepTime >= nextStepTime) {
      nextStepTime = currentStepTime + nextStepTimeOffset;
      currentStep++;
      if ((currentStep % stepLimit) == 0) {
        direction = !direction;
        if (Serial) { Serial.print("Bouncing "); Serial.println(direction); }
        digitalWrite(DIR_PIN, direction ? HIGH : LOW);
        delayMicroseconds(20);

        if (gapWaitTime > 0) {
          nextStepTime += gapWaitTime;
          return;
        }
      }

      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(2);
      digitalWrite(STEP_PIN, LOW);
      delayMicroseconds(2);
    }
  }

  if (nextSerialPollTime <= currentStepTime) {
    if (Serial && Serial.available()) {
      String command = Serial.readStringUntil('\n');
      handleCommand(command);
    }
    nextSerialPollTime = currentStepTime + SERIAL_POLL_INTERVAL;
  }
}

void handleCommand(const String& command) {
  const char chr = command.charAt(0);

  switch (chr) {
    case 'X': case 'x':  stop();                    break;
    case 'O': case 'o':  setStepWaitTime(command);  break;
    case 'C': case 'c':  setCurrent(command);       break;
    case 'D': case 'd':  switchDirection();         break;
    case 'I': case 'i':  switchInterpolation();     break;
    case 'L': case 'l':  setLimit(command);         break;
    case 'G': case 'g':  setGapWaitTime(command);   break;
    case 'M': case 'm':  setMicrostep(command);     break;
    case 'R': case 'r':  readSettings();            break;
  }
}

void stop() {
  if (Serial) Serial.println("Stopping");
  running = false;
}

void setStepWaitTime(const String & command) {
  nextStepTimeOffset = command.substring(1).toInt();
  if (Serial) Serial.print("Setting offset to ");
  if (Serial) Serial.println(nextStepTimeOffset);
  if (nextStepTimeOffset < 20) nextStepTimeOffset = 20;
  else if (nextStepTimeOffset > 10000000) nextStepTimeOffset = 10000000;
  running = true;
}

void setCurrent(const String & command) {
  int current = command.substring(1).toInt();
  if (Serial) Serial.print("Setting current to ");
  if (Serial) Serial.println(current);
  driver.rms_current(current);
}

void switchDirection() {
  direction = !direction;
  if (Serial) Serial.print("Switching direction - ");
  if (Serial) Serial.println(direction);
  digitalWrite(DIR_PIN, direction ? HIGH : LOW);
  delayMicroseconds(4);
  running = true;
}

void switchInterpolation() {
  bool interpolation = !driver.intpol();

  if (Serial) Serial.print("Switching interpolation - ");
  if (Serial) Serial.println(interpolation);
  driver.intpol(interpolation);
  delayMicroseconds(4);
}

void setLimit(const String & command) {
  stepLimit = command.substring(1).toInt();
  if (Serial) Serial.print("Setting limit to ");
  if (Serial) Serial.println(stepLimit);
  if (stepLimit < 1) stepLimit = 1;
  else if (stepLimit > 1000000) stepLimit = 1000000;
  running = true;
}

void setGapWaitTime(const String & command) {
  gapWaitTime = command.substring(1).toInt();
  if (Serial) Serial.print("Setting gap wait time to ");
  if (Serial) Serial.println(gapWaitTime);
  if (gapWaitTime < 0) gapWaitTime = 0;
  else if (gapWaitTime > 100000000) gapWaitTime = 100000000;
  running = true;
}

void setMicrostep(const String & command) {
  int microstep = command.substring(1).toInt();
  if (Serial) Serial.print("Setting microsteps to ");
  if (Serial) Serial.println(microstep);
  if (microstep < 0) microstep = 0;
  else if (microstep > 256) microstep;
  driver.microsteps((uint16_t)microstep);
}

void readSettings() {
  if (!Serial) return;

  uint16_t msread = driver.microsteps();
  Serial.print("Microsteps read: ");
  Serial.println(msread);

  uint8_t mres = driver.mres();
  Serial.print("MRES read: ");
  Serial.println(mres);
  // TMC2209_n::IOIN_t i{ 0 };
  // i.sr = driver.IOIN();
  // TMC2208_n::GCONF_t g{ 0 };
  // g.sr = driver.GCONF();
  // TMC2208_n::CHOPCONF_t c{ 0 };
  // c.sr = driver.CHOPCONF();
  // TMC2209_n::COOLCONF_t l{ 0 };
  // l.sr = driver.COOLCONF();
  // TMC2208_n::PWMCONF_t p{ 0 };
  // p.sr = driver.PWMCONF();
  // uint16_t rmsCurrent = driver.rms_current();

  // displayIoInputs(i);
  // Serial.println("");

  // displayGeneralConfiguration(g);
  // Serial.println("");

  // displayChopperConfiguration(c);
  // Serial.println("");

  // displayCoolStepConfiguration(l);
  // Serial.println("");

  // displayPulseWidthModulationConfiguration(p);
  // Serial.println("");

  // displayRequestedCurrent(rmsCurrent);
}

void displayIoInputs(TMC2209_n::IOIN_t & i) {
  Serial.println("Input IO status:");
  Serial.print("    enn:       ");
  Serial.println(i.enn);
  Serial.print("    ms1:       ");
  Serial.println(i.ms1);
  Serial.print("    ms2:       ");
  Serial.println(i.ms2);
  Serial.print("    diag:      ");
  Serial.println(i.diag);
  Serial.print("    pdn_uart:  ");
  Serial.println(i.pdn_uart);
  Serial.print("    step:      ");
  Serial.println(i.step);
  Serial.print("    spread_en: ");
  Serial.println(i.spread_en);
  Serial.print("    dir:       ");
  Serial.println(i.dir);
  Serial.print("    version:   ");
  Serial.println(i.version);
}

void displayGeneralConfiguration(TMC2208_n::GCONF_t & g) {
  Serial.println("General configuration:");
  Serial.print("    i_scale_analog:   ");
  Serial.println(g.i_scale_analog);
  Serial.print("    internal_rsense:  ");
  Serial.println(g.internal_rsense);
  Serial.print("    en_spreadcycle:   ");
  Serial.println(g.en_spreadcycle);
  Serial.print("    shaft:            ");
  Serial.println(g.shaft);
  Serial.print("    index_otpw:       ");
  Serial.println(g.index_otpw);
  Serial.print("    index_step:       ");
  Serial.println(g.index_step);
  Serial.print("    pdn_disable:      ");
  Serial.println(g.pdn_disable);
  Serial.print("    mstep_reg_select: ");
  Serial.println(g.mstep_reg_select);
  Serial.print("    multistep_filt:   ");
  Serial.println(g.multistep_filt);
}

void displayChopperConfiguration(TMC2208_n::CHOPCONF_t & c) {
  Serial.println("Chopper configuration:");
  Serial.print("    toff:    ");
  Serial.println(c.toff);
  Serial.print("    hstrt:   ");
  Serial.println(c.hstrt);
  Serial.print("    hend:    ");
  Serial.println(c.hend);
  Serial.print("    tbl:     ");
  Serial.println(c.tbl);
  Serial.print("    vsense:  ");
  Serial.println(c.vsense);
  Serial.print("    mres:    ");
  Serial.println(c.mres);
  Serial.print("    intpol:  ");
  Serial.println(c.intpol);
  Serial.print("    dedge:   ");
  Serial.println(c.dedge);
  Serial.print("    diss2g:  ");
  Serial.println(c.diss2g);
  Serial.print("    diss2vs: ");
  Serial.println(c.diss2vs);
}

void displayCoolStepConfiguration(TMC2209_n::COOLCONF_t & l) {
  Serial.println("CoolStep configuration:");
  Serial.print("    semin:  ");
  Serial.println(l.semin);
  Serial.print("    seup:   ");
  Serial.println(l.seup);
  Serial.print("    semax:  ");
  Serial.println(l.semax);
  Serial.print("    sedn:   ");
  Serial.println(l.sedn);
  Serial.print("    seimin: ");
  Serial.println(l.seimin);
}

void displayPulseWidthModulationConfiguration(TMC2208_n::PWMCONF_t & p) {
  Serial.println("Pulse-width modulation configuration:");
  Serial.print("    pwm_ofs:       ");
  Serial.println(p.pwm_ofs);
  Serial.print("    pwm_grad:      ");
  Serial.println(p.pwm_grad);
  Serial.print("    pwm_freq:      ");
  Serial.println(p.pwm_freq);
  Serial.print("    pwm_autoscale: ");
  Serial.println(p.pwm_autoscale);
  Serial.print("    pwm_autograd:  ");
  Serial.println(p.pwm_autograd);
  Serial.print("    freewheel:     ");
  Serial.println(p.freewheel);
  Serial.print("    pwm_reg:       ");
  Serial.println(p.pwm_reg);
  Serial.print("    pwm_lim:       ");
  Serial.println(p.pwm_lim);
}

void displayRequestedCurrent(uint16_t rmsCurrent) {
  Serial.print("Requested current: ");
  Serial.println(rmsCurrent);
}
