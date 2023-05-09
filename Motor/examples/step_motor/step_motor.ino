#include "MotorDriver.h"
uint8_t speed = 0;

StepperMotor motor;

void setup(){
  // Set all motor stop
  
}


void loop(){
  // Set all motor stop
  motor.setSpeed(0, FORWARD,  speed);
  speed = (speed + 10) % 100;
  delay(5000);
}
