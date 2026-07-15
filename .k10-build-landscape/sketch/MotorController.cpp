#line 1 "E:\\projects\\SITP_embeded AI\\SITP_embedded_AI\\hardware\\复制20260714k10\\WeatherStory_TinySelector_K10\\MotorController.cpp"
#include "MotorController.h"

MotorController::MotorController(
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
) :
  leftIn1_(leftIn1),
  leftIn2_(leftIn2),
  rightIn3_(rightIn3),
  rightIn4_(rightIn4),
  leftReversed_(leftReversed),
  rightReversed_(rightReversed),
  initialStopMs_(initialStopMs),
  forwardMs_(forwardMs),
  directionStopMs_(directionStopMs),
  backwardMs_(backwardMs),
  repeatCount_(repeatCount),
  completedCycles_(0),
  state_(MOTION_IDLE),
  stateDeadlineMs_(0) {
}

void MotorController::begin() {
  pinMode(leftIn1_, OUTPUT);
  pinMode(leftIn2_, OUTPUT);
  pinMode(rightIn3_, OUTPUT);
  pinMode(rightIn4_, OUTPUT);
  stop();
}

void MotorController::driveMotor(
  uint8_t in1,
  uint8_t in2,
  int direction,
  bool reversed
) {
  if (reversed) {
    direction = -direction;
  }

  if (direction > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (direction < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

void MotorController::forward() {
  driveMotor(leftIn1_, leftIn2_, 1, leftReversed_);
  driveMotor(rightIn3_, rightIn4_, 1, rightReversed_);
}

void MotorController::backward() {
  driveMotor(leftIn1_, leftIn2_, -1, leftReversed_);
  driveMotor(rightIn3_, rightIn4_, -1, rightReversed_);
}

void MotorController::stopOutputs() {
  driveMotor(leftIn1_, leftIn2_, 0, false);
  driveMotor(rightIn3_, rightIn4_, 0, false);
}

void MotorController::stop() {
  stopOutputs();
  completedCycles_ = 0;
  state_ = MOTION_IDLE;
  stateDeadlineMs_ = 0;
}

void MotorController::enterState(MotionState nextState, unsigned long durationMs) {
  state_ = nextState;
  stateDeadlineMs_ = millis() + durationMs;
}

void MotorController::triggerNewLetterMotion() {
  stop();

  if (repeatCount_ == 0) {
    state_ = MOTION_IDLE;
    return;
  }

  enterState(MOTION_INITIAL_STOP, initialStopMs_);
}

void MotorController::update() {
  if (state_ == MOTION_IDLE) {
    return;
  }

  unsigned long now = millis();
  if ((long)(now - stateDeadlineMs_) < 0) {
    return;
  }

  switch (state_) {
    case MOTION_INITIAL_STOP:
      forward();
      enterState(MOTION_FORWARD, forwardMs_);
      break;

    case MOTION_FORWARD:
      stopOutputs();
      enterState(MOTION_STOP_AFTER_FORWARD, directionStopMs_);
      break;

    case MOTION_STOP_AFTER_FORWARD:
      backward();
      enterState(MOTION_BACKWARD, backwardMs_);
      break;

    case MOTION_BACKWARD:
      stopOutputs();
      enterState(MOTION_STOP_AFTER_BACKWARD, directionStopMs_);
      break;

    case MOTION_STOP_AFTER_BACKWARD:
      completedCycles_++;
      if (completedCycles_ >= repeatCount_) {
        stopOutputs();
        state_ = MOTION_IDLE;
      } else {
        forward();
        enterState(MOTION_FORWARD, forwardMs_);
      }
      break;

    default:
      stop();
      state_ = MOTION_IDLE;
      break;
  }
}

bool MotorController::isActive() const {
  return state_ != MOTION_IDLE;
}
