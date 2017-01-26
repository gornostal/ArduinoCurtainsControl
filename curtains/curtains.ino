
#include <Arduino.h>
#include <Stepper.h>
#include <EtherCard.h>


const int PIN_POT = A1;

namespace MotorCtl {
  const int STEPS_PER_REVOLUTION = 200; // number of steps for the motor to make one revolution
  const int STEPS = 5; // steps per loop iteration
  const int DEFAULT_SPEED = 100;

  const int POWER_PIN = 8;
  
  Stepper myStepper(STEPS_PER_REVOLUTION, 4, 5, 6, 7);
  
  void setup() {
    myStepper.setSpeed(DEFAULT_SPEED);
    pinMode(POWER_PIN, OUTPUT);
  }

  void setSpeed(int speed) {
    myStepper.setSpeed(speed);
  }
  
  void powerOn() {
    digitalWrite(POWER_PIN, HIGH);
  }

  void powerOff() {
    digitalWrite(POWER_PIN, LOW);
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
  const int MAX_TRANSITION_TIME = 10e3;

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
      // this is critical -- if you don't power off your motor, it'll stay in a holding mode,
      // will also drain current and heat up
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

  const int PIN_BUTTON = 9;

  void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
  }

  bool isPressed(){
    return digitalRead(PIN_BUTTON) == LOW;
  }

}


byte Ethernet::buffer[700]; // tcp/ip send and receive buffer

namespace HttpServer {

  const int CS_PIN = 10;

  // ethernet interface ip address
  static byte myip[] = { 192,168,0,200 };
  // gateway ip address
  static byte gwip[] = { 192,168,0,1 };
  // ethernet mac address - must be unique on your network
  static byte mymac[] = { 0x74,0x49,0x69,0x2D,0x30,0x31 };

  const char page[] PROGMEM =
  "HTTP/1.0 200 OK\r\n"
  "Content-Type: text/plain\r\n"
  "\r\n"
  "OK"
  ;

  void setup(){
    if (ether.begin(sizeof Ethernet::buffer, mymac, CS_PIN) == 0) {
      Serial.println( "Failed to access Ethernet controller");
    }

    ether.staticSetup(myip, gwip);

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);
  }

  void loop(){

    // check if valid tcp data is received
    word pos = ether.packetLoop(ether.packetReceive());
    if (pos) {
      char* data = (char *) Ethernet::buffer + pos;

      // HTTP API:
      if (strncmp("GET /stop", data, 9) == 0) {
        Controller::stopTransition();
      } else if (strncmp("GET /open", data, 9) == 0) {
        Controller::openCurtains();
      } else if (strncmp("GET /close", data, 10) == 0) {
        Controller::closeCurtains();
      } else {
        Controller::toggle();
      }

      // send response
      memcpy_P(ether.tcpOffset(), page, sizeof page);
      ether.httpServerReply(sizeof page - 1);
    }

  }

}


void setup() {
  Serial.begin(57600);
  Serial.println("controller setup");
  Controller::setup();
  Serial.println("button setup");
  Button::setup();
  Serial.println("http setup");
  HttpServer::setup();
  Serial.println("setup end");
}

void loop() {
  // set speed
  int speed = map(analogRead(PIN_POT), 0, 1023, 309, 80);
  MotorCtl::setSpeed(speed);
  //Serial.println(speed);

  if (Button::isPressed()) {
    Serial.println("Button pressed");
    Serial.print("Speed ");
    Serial.println(speed);
    Controller::toggle();
    delay(200); // avoid incidental doubleclicks
  }

  Controller::loop();
  HttpServer::loop();
}

