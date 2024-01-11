#include "lineStepCalculator.h"

class PolarPlotter {
private:
  LineStepCalculator lineStepCalculator;
  StepBank steps;
  unsigned int stepIndex;
  unsigned int debugLevel;
  Point position;
  Point finish;
  float maxRadius;
  float radiusStepSize;
  float azimuthStepSize;

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
  PolarPlotter();
  void init(float maxRadius, float radiusStepSize, float azimuthStepSize);
  void computeSteps(String& command);
  bool step();
  void setDebug(unsigned int level);
  static String getHelpMessage();
};
