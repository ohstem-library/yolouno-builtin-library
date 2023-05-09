#include "MotorDriver.h"
#include "Arduino.h"
#include "Wire.h"


DCMotor::DCMotor(){

}

DCMotor::DCMotor(uint8_t address){
    _address = address;
    
}

DCMotor::~DCMotor(){
    
}


bool DCMotor::setSpeed(int motor_index, MotorDirection_t dir,  uint16_t speed){
    if(motor_index < 0 || motor_index >= _number_motor){
        if(Serial){
            Serial.println("Motor index out of range");
        }
        return false;
    }
    //DC MOTOR
    if(speed > 100){
        if(Serial) {
            Serial.println("Speed index out of range");
        }   
        return false;
    }
    data_write[0] = DC_MOTOR;
    data_write[1] = motor_index;
    data_write[2] = dir;
    data_write[3] = speed >> 8;
    data_write[4] = speed & 0xFF;
    Wire.begin();
    Wire.beginTransmission(_address);
    Wire.write(data_write, I2C_DATA_SIZE);
    Wire.endTransmission();
    return true;
}

bool DCMotor::fullOn(int motor_index ,MotorDirection_t dir){
    setSpeed(motor_index, dir, 100);
}

bool DCMotor::fullOff(int motor_index, MotorDirection_t dir){
    setSpeed(motor_index, dir, 0);
}


StepperMotor::StepperMotor(){
    Wire.begin();
}

StepperMotor::StepperMotor(uint8_t address, uint16_t number_step){
    _address = address;
    _number_step = number_step;
    Wire.begin();
}

StepperMotor::~StepperMotor(){

}

bool StepperMotor::setSpeed(int motor_index, MotorDirection_t dir, uint16_t speed, StepperStyle_t style){
    if(motor_index < 0 || motor_index >= _number_motor){
        if(Serial){
            Serial.println("Motor index out of range");
        }
        return false;
    }
    if(speed > 255){
        if(Serial) {
            Serial.println("Speed index out of range");
        }   
        return false;
    }
    if(style > MICROSTEP || style < SINGLE){
        if(Serial) {
            Serial.println("Style is invalid");
        }   
        return false;
    }
    data_write[0] = STEPPER_MOTOR;
    data_write[1] = motor_index;
    data_write[2] = _number_step >> 8;
    data_write[3] = _number_step & 0xFF;
    data_write[4] = style;
    data_write[5] = SPEED;
    data_write[6] = dir;
    data_write[7] = speed >> 8;
    data_write[8] = speed & 0xFF;
    Wire.beginTransmission(_address);        
    Wire.write(data_write, I2C_DATA_SIZE);
    Wire.endTransmission();
    return true;
}

bool StepperMotor::step(int motor_index, MotorDirection_t dir, uint16_t step, StepperStyle_t style){
    if(motor_index < 0 || motor_index >= _number_motor){
        if(Serial){
            Serial.println("Motor index out of range");
        }
        return false;
    }
    if(style > MICROSTEP || style < SINGLE){
        if(Serial) {
            Serial.println("Style is invalid");
        }   
        return false;
    }
    data_write[0] = STEPPER_MOTOR;
    data_write[1] = motor_index;
    data_write[2] = _number_step >> 8;
    data_write[3] = _number_step & 0xFF;
    data_write[4] = style;
    data_write[5] = STEP;
    data_write[6] = dir;
    data_write[7] = step >> 8;
    data_write[8] = step & 0xFF;
    Wire.beginTransmission(_address);        
    Wire.write(data_write, I2C_DATA_SIZE);
    Wire.endTransmission();
    return true;
}

bool StepperMotor::onestep(int motor_index, MotorDirection_t dir, StepperStyle_t style){
    step(motor_index, dir, 1, style);
}

bool StepperMotor::release(uint8_t motor_index){
    setSpeed(motor_index, FORWARD, 0, SINGLE);
}