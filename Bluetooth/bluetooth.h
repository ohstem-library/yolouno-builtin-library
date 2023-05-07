#include "SoftwareSerial.h"

#define BLUETOOTH_DEBUG 1


class BluetoothServer {
    private:
        SoftwareSerial *_serial;
        bool is_connected = false;
        const uint8_t uart_service_index = 1;
        const uint8_t uart_char_rx_index = 1;
        const uint8_t uart_char_tx_index = 2;
        const uint16_t at_timeout = 1000;
        void (*rx_callback)() = NULL;
        char rx_buffer[128];
        size_t rx_buffer_len = 0;
        bool notify_is_enabled = false;
        
        // Internal functions
        bool preprocess_notify_event(bool *notify);
        bool preprocess_write_event(uint8_t *data, size_t * data_len);
    public:
        BluetoothServer();
        BluetoothServer(SoftwareSerial * serial);
        bool restart();
        bool init(const char * device_name = "ESP32");
        bool start_uart_service();
        bool send_data(const char * data);
        bool send_data(uint8_t * data, size_t data_len);
        char* get_receive_data();
        bool set_on_receive_callback(void(*on_receive_callback)());
        void event_loop();
};
