#include "Motor4Drive.h"

Motor4Drive motor_driver(0x30, STEP_MOTOR);

uint8_t speed = 0;

void setup(){
  // Set all motor stop
  motor_driver.set_motors(0);
}


void loop(){
  // Set all motor stop
  motor_driver.set_motor(0, speed);
  speed = (speed + 10) % 250;
  delay(5000);
}
