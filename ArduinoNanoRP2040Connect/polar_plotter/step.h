#include "Arduino.h"

class Step {
  private:
    byte state;
  public:
    Step();
    void setSteps(int radiusStep, int azimuthStep);
    int getRadiusStep();
    int getAzimuthStep();
};
