#include "SoftwareSerial.h"
#include "esp32.h"

SoftwareSerial esp_serial(2,3);
Esp32::Wifi wifi(&esp_serial);
Esp32::Http http(&esp_serial);
uint16_t time_publish;

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
  
}

void loop(){
    // HTTP Get
  if(http.request(Esp32::Http::GET, "http://103.170.122.203/time.php")){
    Serial.print("Get successful: ");
    Serial.println(http.getDataResponse());
  }
  delay(5000);
  // Esp32::event_loop(&esp_serial);
}