#include "SoftwareSerial.h"
#include "esp32.h"

SoftwareSerial esp_serial(2,3);
Esp32::Wifi wifi(&esp_serial);
Esp32::Mqtt mqtt(&esp_serial);
// Esp32::Http http(&serial);
uint16_t time_publish;

void on_message_V1(){
  Serial.print("Message channel V1: ");
  Serial.println(mqtt.getMessageFrom("V1"));
}

void on_message_V2(){
  Serial.print("Message channel V2: ");
  Serial.println(mqtt.getMessageFrom("V2"));
}


void setup(){
  Serial.begin(115200);
  esp_serial.begin(115200);

  // Restart Module
  if(wifi.restart()){
    Serial.println("Module ESP start successful");
  }
  if(wifi.connect_to_ap("Ot Dieu", "abcd@1234")){
    Serial.println("Connected to Wifi");
  }
  // Connect Mqtt Client
  if(mqtt.connect_mqtt_broker("mqtt.ohstem.vn", 1883 ,"eboost-k2", "123456")){
    Serial.println("Mqtt Connected\r\n");
  }
  delay(2000);
  // Subcribe to Topic
  if(mqtt.subcribe_topic("V1", on_message_V1)){
    Serial.println("Subcribed\r\n");
  }
  // Subcribe to Topic
  if(mqtt.subcribe_topic("V2", on_message_V2)){
    Serial.println("Subcribed\r\n");
  }
}

void loop(){
  Esp32::event_loop(&esp_serial);
}