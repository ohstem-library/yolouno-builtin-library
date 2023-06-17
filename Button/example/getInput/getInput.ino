#include "button.h"
#include "scheduler.h"

Button bt(4);

void button_loop(){
  bt.loop();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SCH_Init();
  SCH_Add_Task(button_loop,0,1);
}

void loop() {
  SCH_Dispatch_Tasks();
  // put your main code here, to run repeatedly:
  if(bt.getKeyInput()){
    Serial.print("Key is pressed");
  }  
}
