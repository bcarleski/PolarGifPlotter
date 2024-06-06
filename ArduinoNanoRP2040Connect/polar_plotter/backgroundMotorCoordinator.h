#include <stdio.h>
#include "polarMotorCoordinator.h"
#include "tmcStepDirMotor.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"
#include "safePrinter.h"

#define MINIMUM_INTERVAL 20000
#define MAXIMUM_INTERVAL 1000000000
#define SLOW_STEP_MULTIPLIER 10
#define COMMAND_QUEUE_ENTRIES 10
#define STEP_QUEUE_ENTRIES 100

enum BackgroundAction {
  BACKGROUND_ACTION_CHANGE_STEP_INTERVAL,
  BACKGROUND_ACTION_IS_MOVING,
  BACKGROUND_ACTION_STEP_INTERVAL,
  BACKGROUND_ACTION_CURRENT_POSITION,
  BACKGROUND_ACTION_CURRENT_PROGRESS,
  BACKGROUND_ACTION_CURRENT_STEP,
  BACKGROUND_ACTION_DECLARE_ORIGIN,
  BACKGROUND_ACTION_RESET,
  BACKGROUND_ACTION_PAUSE,
  BACKGROUND_ACTION_RESUME,
  BACKGROUND_ACTION_STOP
};

typedef struct {
  BackgroundAction action;
  unsigned long stepInterval;
} CommandEntry;

typedef struct {
  long radiusStep;
  long azimuthStep;
  bool fastStep;
} StepEntry;

typedef struct {
  bool boolean;
  unsigned long interval;
  long radius;
  long azimuth;
} ResultEntry;

class BackgroundMotorCoordinator : public PolarMotorCoordinator {
private:
  SafePrinter printer;
  PolarMotorCoordinator backgroundCoordinator;
  TmcStepDirMotor radius;
  TmcStepDirMotor azimuth;
  CommandEntry command;
  StepEntry step;
  ResultEntry result;
  Step backgroundPosition;
  Step backgroundProgress;
  Step backgroundStep;
  queue_t commandQueue;
  queue_t stepQueue;
  queue_t resultQueue;

  void addCommandEntry(BackgroundAction action, const unsigned long stepInterval) {
    CommandEntry entry = { action, stepInterval };
    queue_try_add(&commandQueue, &entry);
  }
  void addStepEntry(const long radiusStep, const long azimuthStep, const bool fastStep) {
    StepEntry entry = { radiusStep, azimuthStep, fastStep };
    queue_try_add(&stepQueue, &entry);
  }
  ResultEntry& getResult(BackgroundAction action) {
    addCommandEntry(action, 0UL);
    queue_remove_blocking(&resultQueue, &result);

    return result;
  }
  void sendResult(bool boolean, unsigned long interval, long radius, long azimuth) {
    ResultEntry entry = { boolean, interval, radius, azimuth };
    queue_try_add(&resultQueue, &entry);
  }

public:
  BackgroundMotorCoordinator(SafePrinter printer, const int _radiusDriverAddress, const int _radiusStepPin, const int _radiusDirPin,
                             const int _azimuthDriverAddress, const int _azimuthStepPin, const int _azimuthDirPin)
    : printer(printer),
      radius(TmcStepDirMotor(_radiusDriverAddress, _radiusStepPin, _radiusDirPin)),
      azimuth(TmcStepDirMotor(_azimuthDriverAddress, _azimuthStepPin, _azimuthDirPin)),
      backgroundCoordinator(PolarMotorCoordinator(&radius, &azimuth, 1, MINIMUM_INTERVAL, MAXIMUM_INTERVAL, SLOW_STEP_MULTIPLIER)),
      PolarMotorCoordinator(NULL, NULL, 0, 0, 0, 0.0) {}

  void init() {
    queue_init(&commandQueue, sizeof(CommandEntry), COMMAND_QUEUE_ENTRIES);
    queue_init(&stepQueue, sizeof(StepEntry), STEP_QUEUE_ENTRIES);
    queue_init(&resultQueue, sizeof(ResultEntry), 1);
  }
  void begin() {}
  bool canAddSteps() {
    return !queue_is_full(&stepQueue);
  }
  void changeStepInterval(const unsigned long interval) {
    addCommandEntry(BACKGROUND_ACTION_CHANGE_STEP_INTERVAL, interval);
  }
  void addSteps(const long radiusStep, const long azimuthStep, const bool fastStep) {
    addStepEntry(radiusStep, azimuthStep, fastStep);
  }
  void declareOrigin() {
    addCommandEntry(BACKGROUND_ACTION_DECLARE_ORIGIN, 0UL);
  }
  void move() {}
  void reset() {
    addCommandEntry(BACKGROUND_ACTION_RESET, 0UL);
  }
  void pause() {
    addCommandEntry(BACKGROUND_ACTION_PAUSE, 0UL);
  }
  void resume() {
    addCommandEntry(BACKGROUND_ACTION_RESUME, 0UL);
  }
  void stop() {
    addCommandEntry(BACKGROUND_ACTION_STOP, 0UL);
  }
  bool isMoving() {
    return !queue_is_empty(&stepQueue) || getResult(BACKGROUND_ACTION_IS_MOVING).boolean;
  }

  unsigned long getStepInterval() {
    return getResult(BACKGROUND_ACTION_STEP_INTERVAL).interval;
  }

  Step getCurrentPosition() {
    getResult(BACKGROUND_ACTION_CURRENT_POSITION);
    backgroundPosition.setStepsWithSpeed(result.radius, result.azimuth, result.boolean);
    return backgroundPosition;
  }

  Step getCurrentProgress() {
    getResult(BACKGROUND_ACTION_CURRENT_PROGRESS);
    backgroundProgress.setStepsWithSpeed(result.radius, result.azimuth, result.boolean);
    return backgroundProgress;
  }

  Step getCurrentStep() {
    getResult(BACKGROUND_ACTION_CURRENT_STEP);
    backgroundStep.setStepsWithSpeed(result.radius, result.azimuth, result.boolean);
    return backgroundStep;
  }

  void backgroundInit() {
    backgroundCoordinator.init();
  }
  void backgroundBegin() {
    backgroundCoordinator.begin();
  }
  void backgroundLoop() {
    if (queue_try_remove(&commandQueue, &command)) {
      Step stp;
      bool boolean;
      unsigned long interval;
      long radius;
      long azimuth;

      switch (command.action) {
        case BACKGROUND_ACTION_IS_MOVING:
          boolean = backgroundCoordinator.isMoving();
          sendResult(boolean, 0UL, 0L, 0L);
          break;
        case BACKGROUND_ACTION_STEP_INTERVAL:
          interval = backgroundCoordinator.getStepInterval();
          sendResult(false, interval, 0L, 0L);
          break;
        case BACKGROUND_ACTION_CURRENT_POSITION:
          stp = backgroundCoordinator.getCurrentPosition();
          radius = stp.getRadiusStep();
          azimuth = stp.getAzimuthStep();
          boolean = stp.isFast();
          sendResult(boolean, 0UL, radius, azimuth);
          break;
        case BACKGROUND_ACTION_CURRENT_PROGRESS:
          stp = backgroundCoordinator.getCurrentProgress();
          radius = stp.getRadiusStep();
          azimuth = stp.getAzimuthStep();
          boolean = stp.isFast();
          sendResult(boolean, 0UL, radius, azimuth);
          break;
        case BACKGROUND_ACTION_CURRENT_STEP:
          stp = backgroundCoordinator.getCurrentStep();
          radius = stp.getRadiusStep();
          azimuth = stp.getAzimuthStep();
          boolean = stp.isFast();
          sendResult(boolean, 0UL, radius, azimuth);
          break;
        case BACKGROUND_ACTION_CHANGE_STEP_INTERVAL: backgroundCoordinator.changeStepInterval(command.stepInterval); break;
        case BACKGROUND_ACTION_DECLARE_ORIGIN: backgroundCoordinator.declareOrigin(); break;
        case BACKGROUND_ACTION_RESET: backgroundCoordinator.reset(); break;
        case BACKGROUND_ACTION_PAUSE: backgroundCoordinator.pause(); break;
        case BACKGROUND_ACTION_RESUME: backgroundCoordinator.resume(); break;
        case BACKGROUND_ACTION_STOP: backgroundCoordinator.stop(); break;
      }
    }

    if (backgroundCoordinator.canAddSteps() && queue_try_remove(&stepQueue, &step)) {
      backgroundCoordinator.addSteps(step.radiusStep, step.azimuthStep, step.fastStep);
    }

    backgroundCoordinator.move();
  }
};
