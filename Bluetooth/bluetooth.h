#include "SoftwareSerial.h"

#define BLUETOOTH_DEBUG 1

namespace Esp32 {
    // class BluetoothClient {
    //     private:
    //         SoftwareSerial * _serial;
    //         bool is_connected = false;
    //         char scan_list[4][20] = {0};
    //         uint8_t scan_index = 0;
    //         char gatt_service_list[5][40] = {0};
    //         uint8_t gatt_service_index = 0;
    //         char gatt_characteristic_list[5][40] = {0};
    //         uint8_t gatt_characteristic_index = 0;
    //         char gatt_characteristic_desc_list[3][20] = {0};
    //         uint8_t gatt_characteristic_desc_index = 0;
    //         const uint16_t scan_timeout = 5000; // 5 seconds
    //         const uint16_t process_timeout = 5000; // 5 seconds
    //         const char * GATT_UART_SERVICE = "0x6E400001B5A3F393E0A9E50E24DCCA9E";
    //         const char * GATT_UART_CHAR_RX = "0x6E400003B5A3F393E0A9E50E24DCCA9E";
    //         const char * GATT_UART_CHAR_RX_DESC = "0x2902";
    //         const char * GATT_UART_CHAR_TX = "0x6E400002B5A3F393E0A9E50E24DCCA9E";
    //         uint8_t gatt_uart_service_index;
    //         uint8_t gatt_uart_rx_char_index;
    //         uint8_t gatt_uart_tx_char_index;

    //         // Internal function
    //         bool preprocess_scan_response();
    //         bool preprocess_gatt_service_response();
    //         bool preprocess_gatt_characteristic_response();
    //         bool preprocess_gatt_characteristic_desc_response();
    //     public:
    //         BluetoothClient();
    //         BluetoothClient(SoftwareSerial *serial);
    //         bool init();
    //         bool scan(const char * device_name = NULL);    // Scan and return the device by name or not
    //         uint8_t get_scan_device_len(); // Return length of device scanned
    //         char * get_scan_device_addr(uint8_t index); // Get device address by index in scan list
    //         bool connect(const char * device_name); // Connect to BLuetooth server and check Nordic UART Service
    //         bool check_uart_service(); // Check uart service in Bluetooth Server
    //         bool send_data(uint8_t * data, size_t data_len); // Send Data to Tx characteristic of BLE server
    //         bool receive_data_available(); // Rx Data is available
    //         uint8_t receive_data(); // Receive 1 byte from Rx buffer
    // };

    class BluetoothServer {
        private:
            SoftwareSerial *_serial;
            bool is_connected = false;
            const uint8_t uart_service_index = 0;
            const uint8_t uart_char_tx_index = 0;
            const uint8_t uart_char_rx_index = 1;
            void (*callback)() = NULL;
            char rx_buffer[128];
            uint8_t rx_buffer_len = 0;
        public:
            BluetoothServer();
            BluetoothServer(SoftwareSerial * serial);
            bool init(const char * device_name = "ESP32");
            bool start_uart_service();
            bool send_data(const char * data);
            bool send_data(uint8_t * data, size_t data_len);
            char* get_receive_data();
            bool set_on_receive_callback(void(*on_receive_callback)());
    };
}