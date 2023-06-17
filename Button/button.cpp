#include "stdint.h"
#include "button.h"
#include "Arduino.h"

Button::Button(){

}

Button::~Button(){

}

Button::Button(uint8_t pin){
    this->pin = pin;
    pinMode(pin, INPUT_PULLUP);
}
void Button::loop(){
    uint8_t curr_state = digitalRead(this->pin);
    if(curr_state == this->debounce_state){
        this->state = !curr_state;
        // If before is released
        if(this->state && !this->prev_state){
            // Marked it is pressed
            this->pressed = true;
        }
        this->prev_state = this->state;
    }
    this->debounce_state = curr_state;
}

bool Button::getKeyInput(){
    if(this->pressed){
        this->pressed = false;
        return true;
    }
    return false;
}