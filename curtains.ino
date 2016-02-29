
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
  const int MAX_TRANSITION_TIME = 20e3;

  volatile int movingDir = 0; // -1 closing; 0: stopped; 1: opening
  volatile int lastMovingDir = 0; // -1 closing; 0: stopped; 1: opening
  volatile unsigned long startTransitionMillis = 0;
  

  bool isReedActivated(int reedPin) {
    return digitalRead(reedPin) == LOW;
  }

  bool isClosed() {
    return isReedActivated(PIN_REED_CLOSED);
  }

  bool isOpen() {
    return isReedActivated(PIN_REED_OPEN);
  }
  
  void stopTransition() {
    Serial.println("stopTransition()");
    startTransitionMillis = millis();
    if (movingDir != 0) {
      MotorCtl::powerOff();
      lastMovingDir = movingDir;
      movingDir = 0;
    }
  }

  void onReedClosedInterruption() {
    if (movingDir == -1 && isClosed()) {
      stopTransition();
    }
  }

  void onReedOpenInterruption() {
    if (movingDir == 1 && isOpen()) {
      stopTransition();
    }
  }

  void closeCurtains() {
    Serial.println("closeCurtains()");
    if (movingDir == -1 || isClosed()) {
      // already closing/closed
      return;
    }

    startTransitionMillis = millis();
    lastMovingDir = movingDir;
    movingDir = -1;
  }

  void openCurtains() {
    Serial.println("openCurtains()");
    if (movingDir == 1 || isOpen()) {
      // already opening/open
      return;
    }

    startTransitionMillis = millis();
    lastMovingDir = movingDir;
    movingDir = 1;
  }

  bool toggle() {
    if (movingDir != 0) {
      stopTransition();
    } else if (isClosed()) {
      openCurtains();
    } else if (isOpen()) {
      closeCurtains();
    } else {
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

    // make sure we don't run motor for mor than MAX_TRANSITION_TIME
    // in case one of the reeds gets broken or something
    if (movingDir != 0 && startTransitionMillis > 0 && millis() - startTransitionMillis > MAX_TRANSITION_TIME) {
      stopTransition();
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
    delay(200); // avoid incidental doubleclicks
  }

  Controller::loop();  
}

