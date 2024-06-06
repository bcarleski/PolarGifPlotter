#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/util/queue.h"
#include "pico/multicore.h"

enum BackgroundAction {
  BACKGROUND_ACTION_1,
  BACKGROUND_ACTION_2,
  BACKGROUND_ACTION_3
};

typedef struct {
  BackgroundAction action;
  unsigned long stepInterval;
} CommandEntry;

typedef struct {
  bool boolean;
  unsigned long interval;
} ResultEntry;

queue_t command_queue;
queue_t result_queue;

unsigned long nextCheckTime = 9000;
bool checkResult = false;

void outputTime(unsigned long currentStepTime) {
  if (currentStepTime < 10) Serial.print("          ");
  else if (currentStepTime < 100) Serial.print("         ");
  else if (currentStepTime < 1000) Serial.print("        ");
  else if (currentStepTime < 10000) Serial.print("       ");
  else if (currentStepTime < 100000) Serial.print("      ");
  else if (currentStepTime < 1000000) Serial.print("     ");
  else if (currentStepTime < 10000000) Serial.print("    ");
  else if (currentStepTime < 100000000) Serial.print("   ");
  else if (currentStepTime < 1000000000) Serial.print("  ");
  else Serial.print(" ");
  Serial.print(currentStepTime);
}

void backgroundLoop() {
  CommandEntry commandEntry;

  if (queue_try_remove(&command_queue, &commandEntry)) {
    unsigned long val = commandEntry.stepInterval + commandEntry.action;
    bool isEven = (val % 2) == 0;
    ResultEntry resultEntry = { isEven, val };
    queue_try_add(&result_queue, &resultEntry);
  }
}

void backgroundThreadEntry() {
  unsigned long currentMicros;
  unsigned long nextCheck = 0;

  while (true) {
    backgroundLoop();
    // currentMicros = micros();
    // if (nextCheck <= currentMicros) {
    //   nextCheck += 50000;
    //   backgroundLoop();
    // }
  }
}

void setupBackground() {
  Serial.println("Starting background setup");

  queue_init(&command_queue, sizeof(CommandEntry), 1);
  queue_init(&result_queue, sizeof(ResultEntry), 1);
  Serial.println("Queues initialized");
}

void checkBackground(unsigned long currentStepTime) {
  if (nextCheckTime > currentStepTime) return;

  nextCheckTime = currentStepTime + 5900;

  Serial.print("Checking background - ");
  outputTime(currentStepTime);

  bool success;

  if (checkResult) {
    checkResult = false;
    ResultEntry resultEntry = { false, 0 };
    success = queue_try_remove(&result_queue, &resultEntry);
    if (success) {
      Serial.print(", Got result - ");
      Serial.print(resultEntry.boolean);
      Serial.print(" / ");
      Serial.println(resultEntry.interval);
    } else {
      Serial.println(", No result available");
    }
    return;
  }

  int mod = currentStepTime % 3;
  BackgroundAction action = mod == 0 ? BACKGROUND_ACTION_1 : (mod == 1 ? BACKGROUND_ACTION_2 : BACKGROUND_ACTION_3);
  CommandEntry commandEntry = { action, currentStepTime };
  success = queue_try_add(&command_queue, &commandEntry);
  Serial.print(", Added command ");
  Serial.print(mod);
  Serial.print(" / ");
  Serial.println(success);
  checkResult = true;
}