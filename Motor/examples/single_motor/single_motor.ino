#include "Motor4Drive.h"

Motor4Drive motor_driver(0x30);


void setup(){
  // Set all motor stop
  motor_driver.set_motors(0);
}


void loop(){
  // Set all motor stop
  motor_driver.set_motor(0, 50);
  delay(5000);
}