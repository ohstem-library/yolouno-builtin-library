#include "Motor4Drive.h"
#include "Arduino.h"
#include "Wire.h"

Motor4Drive::Motor4Drive(uint8_t address){
    _address = address;
}

Motor4Drive::~Motor4Drive(){
    
}


bool Motor4Drive::set_motor(int motor_index,int speed){
    if(motor_index < 0 || motor_index >= number_motor){
        if(Serial){
            Serial.println("Motor index out of range");
        }
        return false;
    }
    if(speed < 0 || speed > 100){
        if(Serial){
            Serial.println("Speed index out of range");
        }
        return false;
    }
    Wire.beginTransmission(_address);
    uint8_t data_write[3];
    data_write[0] = motor_index;
    data_write[1] = speed > 0 ? 1 : 0;
    data_write[2] = speed > 0 ? (uint8_t)speed : (uint8_t)(speed * -1);
    Wire.write(data_write, sizeof(data_write));
    Wire.endTransmission(_address);
    return true;
}

bool Motor4Drive::set_motors(int speed){
    for (size_t i = 0; i < number_motor; i++)
    {
        if(!set_motor(i, speed)){
            return false;
        }
    }
    return true;
}