#define SERIAL_POLL_INTERVAL 200000UL
#define POSITION_UPDATE_INTERVAL 2000
#define STEP_UPDATE_INTERVAL 2000
#define RADIUS_STEP_PIN 14
#define RADIUS_DIR_PIN 15
#define RADIUS_ADDRESS 0
#define AZIMUTH_STEP_PIN 16
#define AZIMUTH_DIR_PIN 17
#define AZIMUTH_ADDRESS 1

#include "PolarCoordinateStepper.h"

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

PolarCoordinateStepper stepper(RADIUS_ADDRESS, RADIUS_STEP_PIN, RADIUS_DIR_PIN,
                               AZIMUTH_ADDRESS, AZIMUTH_STEP_PIN, AZIMUTH_DIR_PIN,
                               1);

void setup() {
  stepper.init();

  Serial.begin(115200);
  unsigned long start = millis();

  // Wait for up to 3 seconds for the Serial device to become available
  while (!Serial && (millis() - start) <= 3000)
    ;

  if (Serial) Serial.println("Starting");

  delay(2000);

  stepper.begin();

  delay(2000);

  if (Serial) Serial.println("Finished Setup");

  // readSettings();
  // delay(500);
}

void loop() {
  stepper.move();

  currentStepTime = micros();
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
    case 'P': case 'p':  pause();  break;
    case 'S': case 's':  resume();  break;
    case 'R': case 'r':  reset();  break;
    case 'M': case 'm':  move(command);  break;
    case 'I': case 'i':  setInterpolation(command);  break;
    case 'C': case 'c':  setMicrosteps(command);  break;
    case 'T': case 't':  printStatus();  break;
    // case 'C': case 'c':  setCurrent(command);       break;
    // case 'D': case 'd':  switchDirection();         break;
    // case 'I': case 'i':  switchInterpolation();     break;
    // case 'L': case 'l':  setLimit(command);         break;
    // case 'G': case 'g':  setGapWaitTime(command);   break;
    // case 'M': case 'm':  setMicrostep(command);     break;
    // case 'R': case 'r':  readSettings();            break;
  }
}

void stop() {
  if (Serial) Serial.println("Stopping");
  stepper.stop();
}

void pause() {
  if (Serial) Serial.println("Pausing");
  stepper.pause();
}

void resume() {
  if (Serial) Serial.println("Resuming");
  stepper.resume();
}

void reset() {
  if (Serial) Serial.println("Resetting");
  stepper.reset();
}

void setStepWaitTime(const String & command) {
  nextStepTimeOffset = command.substring(1).toInt();
  if (Serial) Serial.print("Setting offset to ");
  if (Serial) Serial.println(nextStepTimeOffset);
  stepper.changeSpeed(nextStepTimeOffset);
}

void move(const String & command) {
  if (!stepper.canAddSteps()) {
    if (Serial) Serial.println("Not able to add steps currently");
    return;
  }

  String stepCmd = command.substring(1);
  int comma = stepCmd.indexOf(',');
  int radiusSteps = stepCmd.substring(0, comma).toInt();
  int azimuthSteps = stepCmd.substring(comma + 1).toInt();

  if (Serial) Serial.print("Adding move to ");
  if (Serial) Serial.print(radiusSteps);
  if (Serial) Serial.print(" x ");
  if (Serial) Serial.println(azimuthSteps);

  stepper.addSteps(radiusSteps, azimuthSteps);
}

// void setCurrent(const String & command) {
//   int current = command.substring(1).toInt();
//   if (Serial) Serial.print("Setting current to ");
//   if (Serial) Serial.println(current);
//   driver.rms_current(current);
// }

// void switchDirection() {
//   direction = !direction;
//   if (Serial) Serial.print("Switching direction - ");
//   if (Serial) Serial.println(direction);
//   digitalWrite(DIR_PIN, direction ? HIGH : LOW);
//   delayMicroseconds(4);
//   running = true;
// }

// void switchInterpolation() {
//   bool interpolation = !driver.intpol();

//   if (Serial) Serial.print("Switching interpolation - ");
//   if (Serial) Serial.println(interpolation);
//   driver.intpol(interpolation);
//   delayMicroseconds(4);
// }

// void setLimit(const String & command) {
//   stepLimit = command.substring(1).toInt();
//   if (Serial) Serial.print("Setting limit to ");
//   if (Serial) Serial.println(stepLimit);
//   if (stepLimit < 1) stepLimit = 1;
//   else if (stepLimit > 1000000) stepLimit = 1000000;
//   running = true;
// }

// void setGapWaitTime(const String & command) {
//   gapWaitTime = command.substring(1).toInt();
//   if (Serial) Serial.print("Setting gap wait time to ");
//   if (Serial) Serial.println(gapWaitTime);
//   if (gapWaitTime < 0) gapWaitTime = 0;
//   else if (gapWaitTime > 100000000) gapWaitTime = 100000000;
//   running = true;
// }

void setMicrosteps(const String & command) {
  int microstep = command.substring(1).toInt();
  if (Serial) Serial.print("Setting microsteps to ");
  if (Serial) Serial.println(microstep);
  stepper.setMicrosteps(microstep);
}

void setInterpolation(const String & command) {
  int interpolate = command.substring(1).toInt();
  if (Serial) Serial.print("Setting interpolation to ");
  if (Serial) Serial.println(interpolate);
  stepper.setInterpolation(interpolate != 0);
}

void printStatus() {
  stepper.printStatus();
}

// void readSettings() {
//   if (!Serial) return;

//   uint16_t msread = driver.microsteps();
//   Serial.print("Microsteps read: ");
//   Serial.println(msread);

//   uint8_t mres = driver.mres();
//   Serial.print("MRES read: ");
//   Serial.println(mres);
//   // TMC2209_n::IOIN_t i{ 0 };
//   // i.sr = driver.IOIN();
//   // TMC2208_n::GCONF_t g{ 0 };
//   // g.sr = driver.GCONF();
//   // TMC2208_n::CHOPCONF_t c{ 0 };
//   // c.sr = driver.CHOPCONF();
//   // TMC2209_n::COOLCONF_t l{ 0 };
//   // l.sr = driver.COOLCONF();
//   // TMC2208_n::PWMCONF_t p{ 0 };
//   // p.sr = driver.PWMCONF();
//   // uint16_t rmsCurrent = driver.rms_current();

//   // displayIoInputs(i);
//   // Serial.println("");

//   // displayGeneralConfiguration(g);
//   // Serial.println("");

//   // displayChopperConfiguration(c);
//   // Serial.println("");

//   // displayCoolStepConfiguration(l);
//   // Serial.println("");

//   // displayPulseWidthModulationConfiguration(p);
//   // Serial.println("");

//   // displayRequestedCurrent(rmsCurrent);
// }

// void displayIoInputs(TMC2209_n::IOIN_t & i) {
//   Serial.println("Input IO status:");
//   Serial.print("    enn:       ");
//   Serial.println(i.enn);
//   Serial.print("    ms1:       ");
//   Serial.println(i.ms1);
//   Serial.print("    ms2:       ");
//   Serial.println(i.ms2);
//   Serial.print("    diag:      ");
//   Serial.println(i.diag);
//   Serial.print("    pdn_uart:  ");
//   Serial.println(i.pdn_uart);
//   Serial.print("    step:      ");
//   Serial.println(i.step);
//   Serial.print("    spread_en: ");
//   Serial.println(i.spread_en);
//   Serial.print("    dir:       ");
//   Serial.println(i.dir);
//   Serial.print("    version:   ");
//   Serial.println(i.version);
// }

// void displayGeneralConfiguration(TMC2208_n::GCONF_t & g) {
//   Serial.println("General configuration:");
//   Serial.print("    i_scale_analog:   ");
//   Serial.println(g.i_scale_analog);
//   Serial.print("    internal_rsense:  ");
//   Serial.println(g.internal_rsense);
//   Serial.print("    en_spreadcycle:   ");
//   Serial.println(g.en_spreadcycle);
//   Serial.print("    shaft:            ");
//   Serial.println(g.shaft);
//   Serial.print("    index_otpw:       ");
//   Serial.println(g.index_otpw);
//   Serial.print("    index_step:       ");
//   Serial.println(g.index_step);
//   Serial.print("    pdn_disable:      ");
//   Serial.println(g.pdn_disable);
//   Serial.print("    mstep_reg_select: ");
//   Serial.println(g.mstep_reg_select);
//   Serial.print("    multistep_filt:   ");
//   Serial.println(g.multistep_filt);
// }

// void displayChopperConfiguration(TMC2208_n::CHOPCONF_t & c) {
//   Serial.println("Chopper configuration:");
//   Serial.print("    toff:    ");
//   Serial.println(c.toff);
//   Serial.print("    hstrt:   ");
//   Serial.println(c.hstrt);
//   Serial.print("    hend:    ");
//   Serial.println(c.hend);
//   Serial.print("    tbl:     ");
//   Serial.println(c.tbl);
//   Serial.print("    vsense:  ");
//   Serial.println(c.vsense);
//   Serial.print("    mres:    ");
//   Serial.println(c.mres);
//   Serial.print("    intpol:  ");
//   Serial.println(c.intpol);
//   Serial.print("    dedge:   ");
//   Serial.println(c.dedge);
//   Serial.print("    diss2g:  ");
//   Serial.println(c.diss2g);
//   Serial.print("    diss2vs: ");
//   Serial.println(c.diss2vs);
// }

// void displayCoolStepConfiguration(TMC2209_n::COOLCONF_t & l) {
//   Serial.println("CoolStep configuration:");
//   Serial.print("    semin:  ");
//   Serial.println(l.semin);
//   Serial.print("    seup:   ");
//   Serial.println(l.seup);
//   Serial.print("    semax:  ");
//   Serial.println(l.semax);
//   Serial.print("    sedn:   ");
//   Serial.println(l.sedn);
//   Serial.print("    seimin: ");
//   Serial.println(l.seimin);
// }

// void displayPulseWidthModulationConfiguration(TMC2208_n::PWMCONF_t & p) {
//   Serial.println("Pulse-width modulation configuration:");
//   Serial.print("    pwm_ofs:       ");
//   Serial.println(p.pwm_ofs);
//   Serial.print("    pwm_grad:      ");
//   Serial.println(p.pwm_grad);
//   Serial.print("    pwm_freq:      ");
//   Serial.println(p.pwm_freq);
//   Serial.print("    pwm_autoscale: ");
//   Serial.println(p.pwm_autoscale);
//   Serial.print("    pwm_autograd:  ");
//   Serial.println(p.pwm_autograd);
//   Serial.print("    freewheel:     ");
//   Serial.println(p.freewheel);
//   Serial.print("    pwm_reg:       ");
//   Serial.println(p.pwm_reg);
//   Serial.print("    pwm_lim:       ");
//   Serial.println(p.pwm_lim);
// }

// void displayRequestedCurrent(uint16_t rmsCurrent) {
//   Serial.print("Requested current: ");
//   Serial.println(rmsCurrent);
// }
