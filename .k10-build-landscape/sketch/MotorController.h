#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\MotorController.h"
#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

class MotorController {
 public:
  MotorController(
    uint8_t leftIn1,
    uint8_t leftIn2,
    uint8_t rightIn3,
    uint8_t rightIn4,
    bool leftReversed,
    bool rightReversed,
    unsigned long initialStopMs,
    unsigned long forwardMs,
    unsigned long directionStopMs,
    unsigned long backwardMs,
    uint8_t repeatCount
  );

  void begin();
  void forward();
  void backward();
  void stop();
  void triggerNewLetterMotion();
  void update();
  bool isActive() const;

 private:
  enum MotionState {
    MOTION_IDLE,
    MOTION_INITIAL_STOP,
    MOTION_FORWARD,
    MOTION_STOP_AFTER_FORWARD,
    MOTION_BACKWARD,
    MOTION_STOP_AFTER_BACKWARD
  };

  void driveMotor(uint8_t in1, uint8_t in2, int direction, bool reversed);
  void stopOutputs();
  void enterState(MotionState nextState, unsigned long durationMs);

  uint8_t leftIn1_;
  uint8_t leftIn2_;
  uint8_t rightIn3_;
  uint8_t rightIn4_;
  bool leftReversed_;
  bool rightReversed_;
  unsigned long initialStopMs_;
  unsigned long forwardMs_;
  unsigned long directionStopMs_;
  unsigned long backwardMs_;
  uint8_t repeatCount_;
  uint8_t completedCycles_;
  MotionState state_;
  unsigned long stateDeadlineMs_;
};

#endif
