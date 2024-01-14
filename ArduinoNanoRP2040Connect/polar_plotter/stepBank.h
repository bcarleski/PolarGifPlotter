#include "condOut.h"
#include "step.h"

// This should be greater than the following calculation:
//     MAX_RADIUS / (RADIUS_STEP_SIZE)
#define MAX_STEPS 8192

class StepBank {
private:
  CondOut condOut;
  Step blankStep;
  Step steps[MAX_STEPS];
  unsigned int stepCount;
  unsigned int debugLevel;
public:
  StepBank(CondOut& condOut);
  void addStep(int radiusStep, int azimuthStep);
  Step getStep(unsigned int stepIndex);
  unsigned int getStepCount();
  void reset();
  void setDebug(unsigned int level);
};
