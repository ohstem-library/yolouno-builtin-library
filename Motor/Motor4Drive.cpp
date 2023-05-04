#include "Motor4Drive.h"
#include "Arduino.h"
#include "Wire.h"

Motor4Drive::Motor4Drive(uint8_t address){
    _address = address;
    _motor_type = DC_MOTOR;
    Wire.begin();
}

Motor4Drive::Motor4Drive(uint8_t address, uint8_t motor_type = DC_MOTOR){
    _address = address;
    _motor_type = motor_type;
    if(motor_type == DC_MOTOR){
        _number_motor = 4;
    }else{
        _number_motor = 2;
    }
    Wire.begin();
}

Motor4Drive::~Motor4Drive(){
    
}


bool Motor4Drive::set_motor(int motor_index,int speed){
    if(motor_index < 0 || motor_index >= _number_motor){
        if(Serial){
            Serial.println("Motor index out of range");
        }
        return false;
    }
    //DC MOTOR
    if (_motor_type == DC_MOTOR) { 
        if(speed < -100 || speed > 100){
            if(Serial) {
                Serial.println("Speed index out of range");
            }   
            return false;
        }
        data_write[0] = DC_MOTOR;
        data_write[1] = motor_index;
        data_write[2] = speed > 0 ? 1 : 0;
        data_write[3] = speed >= 0 ? (uint8_t)speed : (uint8_t)(speed * -1);
        Wire.beginTransmission(_address);
        Wire.write(data_write, sizeof(data_write));
        Wire.endTransmission();
        return true;
    }
    else if (_motor_type == STEP_MOTOR) {     //STEP MOTOR
        if(speed < -255 || speed > 255){
            if(Serial) {
                Serial.println("Speed index out of range");
            }   
            return false;
        }
        data_write[0] = STEP_MOTOR;
        data_write[1] = motor_index;
        data_write[2] = speed > 0 ? 1 : 0;
        data_write[3] = speed >= 0 ? (uint8_t)speed : (uint8_t)(speed * -1);
        Wire.beginTransmission(_address);        
        Wire.write(data_write, sizeof(data_write));
        Wire.endTransmission();
        return true;
    }
}

bool Motor4Drive::set_motors(int speed){
    for (size_t i = 0; i < _number_motor; i++)
    {
        if(!set_motor(i, speed)){
            return false;
        }
    }
    return true;
}