#define USE_LCD 1
#define USE_CLOUD 1

#define LCD_RS 7
#define LCD_EN 6
#define LCD_D4 12
#define LCD_D5 11
#define LCD_D6 10
#define LCD_D7 9

#define RADIUS_STEPPER_STEPS_PER_ROTATION 200
#define RADIUS_STEPPER_STEP_PIN 14
#define RADIUS_STEPPER_DIR_PIN 15
#define RADIUS_GEAR_RATIO 1.0                  // # of drive gear teeth / # of motor gear teeth, i.e. if the drive shaft gear has 20 teeth, and the motor has 10, this should be 2.0
#define RADIUS_ROTATIONS_TO_MAX_RADIUS 3.0     // the number of complete turns of the drive gear that it takes to go from the center to the maximum radius
#define RADIUS_RPMS 36                         // 60 * RADIUS_ROTATIONS_TO_MAX_RADIUS * RADIUS_GEAR_RATIO / {secondsToGoFromCenterToEdge=5}

#define AZIMUTH_STEPPER_STEPS_PER_ROTATION 200
#define AZIMUTH_STEPPER_STEP_PIN 16
#define AZIMUTH_STEPPER_DIR_PIN 17
#define AZIMUTH_GEAR_RATIO 1.0                 // # of drive gear teeth / # of motor gear teeth, i.e. if the drive shaft gear has 20 teeth, and the motor has 10, this should be 2.0
#define AZIMUTH_RPMS 6                         // 60 * AZIMUTH_GEAR_RATIO / {secondsForOneCompleteRotation=10}

#define MAX_RADIUS 600
#define RADIUS_STEP_SIZE MAX_RADIUS / RADIUS_ROTATIONS_TO_MAX_RADIUS / RADIUS_STEPPER_STEPS_PER_ROTATION / RADIUS_GEAR_RATIO
#define AZIMUTH_STEP_SIZE 2 * PI / AZIMUTH_STEPPER_STEPS_PER_ROTATION / AZIMUTH_GEAR_RATIO

#define MARBLE_SIZE_IN_RADIUS_STEPS 20
