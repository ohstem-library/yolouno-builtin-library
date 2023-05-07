#include "bluetooth.h"
#include "SoftwareSerial.h"

SoftwareSerial serial(6,5);
Esp32::BluetoothServer server(&serial);

void rx_callback(){
    Serial.print("Rx data: ");
    Serial.println(server.get_receive_data());
}

void setup(){
    Serial.begin(115200);
    serial.begin(9600);
    // Init
    server.init("Esp32");
    // Start Uart Service
    server.start_uart_service();
    // Set receive callback
    server.set_on_receive_callback(rx_callback);
}


void loop(){
    server.send_data("Hello world from Bluetooth Server");
    delay(2000);
}