#include "SoftwareSerial.h"

#define BLUETOOTH_DEBUG 1


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
        void event_loop();
};
