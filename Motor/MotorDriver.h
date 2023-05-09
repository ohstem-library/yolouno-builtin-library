#ifndef MOTOR4DRIVE_H
#define MOTOR4DRIVE_H

#include "stdint.h"

#define I2C_DATA_SIZE   9

typedef enum {
    DC_MOTOR = 0,
    STEPPER_MOTOR
}MotorType_t;

typedef enum {
    FORWARD,
    BACKWARD
}MotorDirection_t;


class DCMotor {
    private:
        uint8_t _address = 0x30;
        uint8_t _number_motor = 4;
        uint8_t data_write[I2C_DATA_SIZE];
    public:
        DCMotor();
        DCMotor(uint8_t address);
        ~DCMotor();
        bool setSpeed(int motor_index, MotorDirection_t dir,  uint16_t speed);
        bool fullOn(int motor_index,  MotorDirection_t dir);
        bool fullOff(int motor_index,  MotorDirection_t dir);
};   

class StepperMotor {

    private:
        uint8_t _address = 0x30;
        uint8_t _number_motor = 2;
        uint16_t _number_step = 200;
        uint8_t data_write[I2C_DATA_SIZE];

    public:
        typedef enum {
            SINGLE = 0,
            DOUBLE,
            INTERLEAVE,
            MICROSTEP
        }StepperStyle_t;

        typedef enum {
            SPEED = 0,
            STEP
        }StepperMode_t;

        StepperMotor();
        StepperMotor(uint8_t address, uint16_t number_step = 200);
        ~StepperMotor();
        bool setSpeed(int motor_index, MotorDirection_t dir, uint16_t speed, StepperStyle_t style = SINGLE);
        bool step(int motor_index, MotorDirection_t dir, uint16_t step, StepperStyle_t style = SINGLE);
        bool onestep(int motor_index, MotorDirection_t dir, StepperStyle_t style = SINGLE);
        bool release(uint8_t motor_index);
};

#endif // MOTOR4DRIVE_H