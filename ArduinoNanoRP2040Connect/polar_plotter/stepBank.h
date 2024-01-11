#define MAX_STEPS 8192
#include "step.h"

class StepBank {
private:
  Step blankStep;
  Step steps[MAX_STEPS];
  unsigned int stepCount;
public:
  StepBank();
  void addStep(int radiusStep, int azimuthStep);
  Step getStep(unsigned int stepIndex);
  unsigned int getStepCount();
  void reset();
};
