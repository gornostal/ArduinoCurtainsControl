#include "Motor.ino"

namespace Controller {

  const int PIN_REED_CLOSED = A0;
  const int PIN_REED_OPEN = A1;

  enum class State {UNKNOWN, OPEN, CLOSED, IN_TRANSITION};

  volatile State currentState = State::UNKNOWN;
  volatile int movingDir = 0; // -1 closing; 0: stopped; 1: opening
  volatile int lastMovingDir = 0; // -1 closing; 0: stopped; 1: opening

  bool isReedActivated(int reedPin) {
    return digitalRead(reedPin) == LOW;
  }

  void onReedClosedInterruption() {
    if (movingDir == -1 && isReedActivated(PIN_REED_CLOSED)) {
      // TODO: stop motor
      lastMovingDir = movingDir;
      movingDir = 0;
      currentState = State::CLOSED;
    }
  }

  void onReedOpenInterruption() {
    if (movingDir == 1 && isReedActivated(PIN_REED_OPEN)) {
      // TODO: stop motor
      lastMovingDir = movingDir;
      movingDir = 0;
      currentState = State::CLOSED;
    }
  }

  bool closeCurtains() {
    if (movingDir == -1) {
      // already closing
      return false;
    }

    // TODO: run motor backward
    lastMovingDir = movingDir;
    movingDir = -1;
    currentState = State::IN_TRANSITION;
    return true;
  }

  bool openCurtains() {
    if (movingDir == 1) {
      // already opening
      return false;
    }

    // TODO: run motor forward
    lastMovingDir = movingDir;
    movingDir = 1;
    currentState = State::IN_TRANSITION;
    return true;
  }

  void stopTransition() {
    if (currentState == State::IN_TRANSITION) {
      // TODO: stop motor
      lastMovingDir = movingDir;
      movingDir = 0;
      currentState = State::UNKNOWN;
    }
  }

  bool toggle() {
    switch(currentState){
      case State::OPEN:
        closeCurtains();
        break;
      case State::CLOSED:
        openCurtains();
        break;
      case State::IN_TRANSITION:
        stopTransition();
        break;
      default:
        if (lastMovingDir == -1) {
          openCurtains();
        } else {
          closeCurtains();
        }
    }
  }

  void setup() {
    pinMode(PIN_REED_OPEN, INPUT);
    pinMode(PIN_REED_CLOSED, INPUT);
    digitalWrite(PIN_REED_OPEN, HIGH);    // Activate internal pullup resistor
    digitalWrite(PIN_REED_CLOSED, HIGH);  // Activate internal pullup resistor
    attachInterrupt(digitalPinToInterrupt(PIN_REED_OPEN), onReedOpenInterruption, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_REED_CLOSED), onReedClosedInterruption, CHANGE);

    // TODO: setup motor
  }

}
