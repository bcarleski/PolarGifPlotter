#include <TMCStepper.h>
#include "stepDirMotor.h"
#include "pico/time.h"

#define SERIAL_PORT Serial1  // HardwareSerial port pins 0 & 1
#define R_SENSE 0.11f

void delayMicros(unsigned long micros) {
  sleep_us(micros);
}

class TmcStepDirMotor : public StepDirMotor {
private:
  TMC2209Stepper driver;

protected:
  void initDriver() {
    SERIAL_PORT.begin(115200);
  }
  void beginDriver() {
    driver.begin();
    driver.reset();
    driver.toff(5);
    driver.microsteps(0);
    driver.intpol(true);
  }

public:
  TmcStepDirMotor(const int _driverAddress, const int _stepPin, const int _dirPin)
    : driver(TMC2209Stepper(&SERIAL_PORT, R_SENSE, _driverAddress)),
      StepDirMotor(_stepPin, _dirPin, delayMicros) {}
};
