#include "Arduino.h"

class Step {
  private:
    byte state;
  public:
    Step();
    Step(int radiusStep, int azimuthStep);
    int getRadiusStep();
    int getAzimuthStep();
};
