#include "bluetooth.h"
#include "SoftwareSerial.h"

SoftwareSerial serial(6,5);
BluetoothServer server(&serial);

void rx_callback(){
    Serial.print("Rx data: ");
    Serial.println(server.get_receive_data());
}

void setup(){
    Serial.begin(115200);
    serial.begin(9600);
    // Restart module
    server.restart();
    // Init
    server.init("Esp32");
    // Start Uart Service
    server.start_uart_service();
    // Set receive callback
    server.set_on_receive_callback(rx_callback);
}

uint16_t time_to_send = millis();

void loop(){
    if(millis() - time_to_send > 2000){
      time_to_send = millis();
      server.send_data("Hello world");
    }
    server.event_loop();
}
