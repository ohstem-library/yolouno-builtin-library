#ifndef MOTOR4DRIVE_H
#define MOTOR4DRIVE_H

#include "stdint.h"

class Motor4Drive {
    private:
        uint8_t _address;
        const uint8_t number_motor = 4;
    public:
        Motor4Drive(uint8_t address);
        ~Motor4Drive();
        bool set_motor(int motor_index, int speed);
        bool set_motors(int speed);
};

#endif // MOTOR4DRIVE_H