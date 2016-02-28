#include <Stepper.h>

namespace Motor {
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

