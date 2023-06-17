#ifndef BUTTON_H
#define BUTTON_H

#include "stdint.h"

class Button{
    private:
        uint8_t pin;
        uint8_t debounce_state = false; 
        uint8_t prev_state = false;
        uint8_t state = false;
        uint8_t pressed = false;
    public:
        Button();
        ~Button();
        Button(uint8_t pin);
        void loop();
        bool getKeyInput();
};

#endif // BUTTON_H