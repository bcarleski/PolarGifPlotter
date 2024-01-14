#include "lineStepCalculator.h"
#include <Stepper.h>

class PolarPlotter {
private:
  CondOut condOut;
  Stepper radiusStepper;
  Stepper azimuthStepper;
  LineStepCalculator lineStepCalculator;
  void (*publishMessage)(String&, String&, int);
  StepBank steps;
  unsigned int stepIndex;
  unsigned int debugLevel;
  unsigned long stepStartMillis;
  Point position;
  Point finish;
  float maxRadius;
  float radiusStepSize;
  float azimuthStepSize;

  void executeStepsToCenter();
  void executeRadiusSteps(int radiusSteps);
  void executeFullCircleSteps();
  void setFinishPoint(String& command) {
    int commaIndex = command.indexOf(',');
    float rawX = command.substring(1, commaIndex).toFloat();
    float rawY = command.substring(commaIndex + 1).toFloat();
    float rawR = sqrt(rawX * rawX + rawY * rawY);
    float rawA = atan2(rawY, rawX);
    float radius = round(rawR / this->radiusStepSize) * this->radiusStepSize;
    float azimuth = round(rawA / this->azimuthStepSize) * this->azimuthStepSize;

    this->finish.repoint(radius, azimuth);
  }


public:
  PolarPlotter(CondOut&, Stepper&, Stepper&, float, float, float);
  void init(float, float);
  void computeSteps(String&);
  void executeWipe();
  bool step();
  unsigned int getStepCount();
  Point getPosition();
  void setDebug(unsigned int);
  void onPublishMessage(void messagePublisher(String&, String&, int));
  static String getHelpMessage();
};
