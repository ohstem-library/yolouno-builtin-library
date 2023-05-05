#include "Motor4Drive.h"

Motor4Drive motor_driver(0x30, DC_MOTOR);

uint8_t speed = 0;

void setup(){
  // Set all motor stop
  motor_driver.set_motors(0);
}


void loop(){
  // Set all motor stop
  motor_driver.set_motor(0, speed);
  speed = (speed + 10) % 100;
  delay(5000);
}
