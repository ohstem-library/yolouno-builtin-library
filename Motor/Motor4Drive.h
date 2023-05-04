#ifndef MOTOR4DRIVE_H
#define MOTOR4DRIVE_H

#define DC_MOTOR 0
#define STEP_MOTOR 1

#include "stdint.h"

class Motor4Drive {
    private:
        uint8_t _address;
        const uint8_t number_motor = 4;
        uint8_t _motor_type;
    public:
        Motor4Drive(uint8_t address);
        Motor4Drive(uint8_t address, uint8_t motor_type = DC_MOTOR);
        ~Motor4Drive();
        bool set_motor(int motor_index, int speed);
        bool set_motors(int speed);
};

#endif // MOTOR4DRIVE_H