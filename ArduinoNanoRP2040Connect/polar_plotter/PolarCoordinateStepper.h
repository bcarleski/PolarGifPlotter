#include "PolarStepper.h"

#define MAX_POLAR_STEPS 10
#define LOWEST_MINIMUM_OFFSET 20000
#define SLOW_STEP_MULTIPLIER 10

class PolarCoordinateStepper {
private:
  PolarStepper radius;
  PolarStepper azimuth;

  // Interlocked != 0 means that azimuth and radius are physically connected, such that turning the azimuth results in a change in radius
  // A positive value indicates that one step of the azimuth results in one or more steps of the radius in the same direction (value of interlocked is the # of steps)
  // A negative value indicates that one step of the azimuth results in one or more steps of the radius in the opposite direction
  int interlocked;
  unsigned long minimumOffset = LOWEST_MINIMUM_OFFSET;
  int radiusSteps[MAX_POLAR_STEPS];
  int azimuthSteps[MAX_POLAR_STEPS];
  bool fastSteps[MAX_POLAR_STEPS];
  int paused = false;
  int moving = false;
  int movingIndex = 0;
  int savingIndex = 1;

protected:
  int getNextIndex(int index);
  bool prepareMove();
  void recalculateMove();
  bool setupMove(const int nextRadiusSteps, const int nextAzimuthSteps, const bool fastStep);

public:
  PolarCoordinateStepper(const int _radiusDriverAddress, const int _radiusStepPin, const int _radiusDirPin,
                         const int _azimuthDriverAddress, const int _azimuthStepPin, const int _azimuthDirPin,
                         const int _interlocked);
  void init();
  void begin();
  bool canAddSteps();
  bool hasSteps();
  void changeSpeed(const unsigned long offset);
  void addSteps(const int radiusStep, const int azimuthStep, const bool fastStep);
  void declareOrigin();
  void move();
  void reset();
  void pause();
  void resume();
  void stop();
};