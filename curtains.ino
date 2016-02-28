
#include <Arduino.h>
#include <Stepper.h>


namespace MotorCtl {
  const int STEPS_PER_REVOLUTION = 200;
  const int STEPS = 10; // steps per interruption check
  const int SPEED = 60;

  const int SPPED_PIN_1 = 9;
  const int SPPED_PIN_2 = 8;
  //const int SPPED_PIN_1 = 9;
  //const int SPPED_PIN_2 = 10;

  Stepper myStepper(STEPS_PER_REVOLUTION, 4, 5, 7, 6);
  //Stepper myStepper(STEPS_PER_REVOLUTION, 8, 11, 12, 13);

  void setup() {
    myStepper.setSpeed(SPEED);
    pinMode(SPPED_PIN_1, OUTPUT);
    pinMode(SPPED_PIN_2, OUTPUT);
  }

  void powerOn() {
    digitalWrite(SPPED_PIN_1, HIGH);
    digitalWrite(SPPED_PIN_2, HIGH);
  }
  
  void powerOff() {
    digitalWrite(SPPED_PIN_1, LOW);
    digitalWrite(SPPED_PIN_2, LOW);
  }

  void stepForward() {
    powerOn();
    myStepper.step(STEPS);
  }

  void stepBackward() {
    powerOn();
    myStepper.step(-STEPS);
  }

}


namespace Controller {

  const int PIN_REED_CLOSED = 3;
  const int PIN_REED_OPEN = 2;

  enum class State {UNKNOWN, OPEN, CLOSED, IN_TRANSITION};

  volatile State currentState = State::UNKNOWN;
  volatile int movingDir = 0; // -1 closing; 0: stopped; 1: opening
  volatile int lastMovingDir = 0; // -1 closing; 0: stopped; 1: opening

  bool isReedActivated(int reedPin) {
    return digitalRead(reedPin) == LOW;
  }

  void stopTransition() {
    if (currentState == State::IN_TRANSITION) {
      MotorCtl::powerOff();
      lastMovingDir = movingDir;
      movingDir = 0;
      currentState = State::UNKNOWN;
    }
  }

  void onReedClosedInterruption() {
    Serial.print("onReedClosedInterruption ");
    Serial.println(movingDir);
    if (movingDir == -1 && isReedActivated(PIN_REED_CLOSED)) {
      stopTransition();
      currentState = State::CLOSED;
    }
  }

  void onReedOpenInterruption() {
    Serial.print("onReedOpenInterruption ");
    Serial.println(movingDir);
    if (movingDir == 1 && isReedActivated(PIN_REED_OPEN)) {
      stopTransition();
      currentState = State::CLOSED;
    }
  }

  bool closeCurtains() {
    if (movingDir == -1) {
      // already closing
      return false;
    }

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

    lastMovingDir = movingDir;
    movingDir = 1;
    currentState = State::IN_TRANSITION;
    return true;
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

    pinMode(PIN_REED_OPEN, INPUT_PULLUP);
    pinMode(PIN_REED_CLOSED, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_REED_OPEN), onReedOpenInterruption, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_REED_CLOSED), onReedClosedInterruption, CHANGE);    
  }

  void loop() {
    if (movingDir == -1) {
      MotorCtl::stepBackward();
    } else if (movingDir == 1) {
      MotorCtl::stepForward();
    }
  }

}


namespace Button {

  const int PIN_BUTTON = 14;

  void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
  }

  bool isPressed(){
    return digitalRead(PIN_BUTTON) == LOW;
  }
  
}


void setup() {
  Serial.begin(9600);
  Controller::setup();
  Button::setup();
}

void loop() {
  if (Button::isPressed()) {
    Serial.println("Button pressed");
    Controller::toggle();
    delay(100); // avoid incidental doubleclicks
  }

  Controller::loop();  
}

