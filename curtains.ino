
#include <Arduino.h>
#include <Stepper.h>

namespace MotorCtl {
  const int STEPS_PER_REVOLUTION = 200;
  const int RUN_INTERVAL = 30; // steps per interruption check
  const int SPEED = 60;

  const int SPPED_PIN_1 = 6;
  const int SPPED_PIN_2 = 3;
  //const int SPPED_PIN_1 = 9;
  //const int SPPED_PIN_2 = 10;

  Stepper myStepper(STEPS_PER_REVOLUTION, 2, 5, 4, 7);
  //Stepper myStepper(STEPS_PER_REVOLUTION, 8, 11, 12, 13);

  volatile bool interrupted = false;

  void setup() {
    myStepper.setSpeed(SPEED);

    pinMode(SPPED_PIN_1, OUTPUT);
    pinMode(SPPED_PIN_2, OUTPUT);
    digitalWrite(SPPED_PIN_1, HIGH);
    digitalWrite(SPPED_PIN_2, HIGH);
  }

  void runForward() {
    interrupted = false;
    while (!interrupted) {
      myStepper.step(RUN_INTERVAL);
    }
  }

  void runBackward() {
    interrupted = false;
    while (!interrupted) {
      myStepper.step(-RUN_INTERVAL);
    }
  }

  void stop() {
    interrupted = true;
  }

}




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
      MotorCtl::stop();
      lastMovingDir = movingDir;
      movingDir = 0;
      currentState = State::CLOSED;
    }
  }

  void onReedOpenInterruption() {
    if (movingDir == 1 && isReedActivated(PIN_REED_OPEN)) {
      MotorCtl::stop();
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

    MotorCtl::runBackward();
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

    MotorCtl::runForward();
    lastMovingDir = movingDir;
    movingDir = 1;
    currentState = State::IN_TRANSITION;
    return true;
  }

  void stopTransition() {
    if (currentState == State::IN_TRANSITION) {
      MotorCtl::stop();
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
    MotorCtl::setup();    

    pinMode(PIN_REED_OPEN, INPUT);
    pinMode(PIN_REED_CLOSED, INPUT);
    digitalWrite(PIN_REED_OPEN, HIGH);    // Activate internal pullup resistor
    digitalWrite(PIN_REED_CLOSED, HIGH);  // Activate internal pullup resistor
    attachInterrupt(digitalPinToInterrupt(PIN_REED_OPEN), onReedOpenInterruption, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_REED_CLOSED), onReedClosedInterruption, CHANGE);    
  }

}


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
