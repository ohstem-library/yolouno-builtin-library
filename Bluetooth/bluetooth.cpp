#include "Arduino.h"
#include "bluetooth.h"
#include "string.h"

typedef enum AtResponse{
    AT_GENERIC_OK = 0,
    AT_GENERIC_ERROR,
    AT_GENERIC_INPUT,
    // System Response
    AT_SYSTEM_STARTUP,
    // Bluetooth Response
    AT_BLE_NOTIFY_ENABLED,
    AT_BLE_RX,
    AT_MAX
};

static const uint16_t AT_TIMEOUT = 10000; // 500ms
static const uint16_t AT_BUFFER_MAX_SIZE = 128; // 128 bytes
static char AT_BUFFER[AT_BUFFER_MAX_SIZE] = {0};
static char AT_BUFFER_RESPONSE[AT_BUFFER_MAX_SIZE] = {0};

static const uint8_t AT_RESPONSE_SIZE = AT_MAX;

static const char * AT_RESPONSE_TABLE[AT_MAX] = {
    [AT_GENERIC_OK] = "OK",
    [AT_GENERIC_ERROR] = "ERROR",
    [AT_GENERIC_INPUT] = ">",
    // System Response
    [AT_SYSTEM_STARTUP] = "ready",
    // Bluetooth Response
    [AT_BLE_NOTIFY_ENABLED] = "+WRITE:0,1,2,1,2,",
    [AT_BLE_RX] = "+WRITE:0,1,1,,"
};
static AtResponse persistent_response = AT_MAX;

static bool find(char *buffer , uint16_t buffer_len , const char * data);
static AtResponse wait_response(SoftwareSerial * _serial, uint32_t timeout = AT_TIMEOUT);
static AtResponse wait_response_persistent(SoftwareSerial * _serial);
static bool wait_response(SoftwareSerial * _serial, AtResponse expected_response, uint32_t timeout = AT_TIMEOUT);
static void flush(SoftwareSerial * _serial, uint32_t timeout_ms);
// Event
void event_loop(SoftwareSerial *_serial);

/* *****************************************Utils******************************************/
static bool find(char *buffer , uint16_t buffer_len , const char * data){
    if(buffer == NULL || data == NULL){
        return false;
    }
    uint16_t data_len = strlen(data);
    uint16_t actual_buffer_len = strlen(buffer);
    // Validate valid buffer
    if(actual_buffer_len == 0 || buffer_len < data_len || data_len == 0){
        return false;
    }
    else{
        for (uint16_t index = 0; index < data_len; index++) {
            if(buffer[buffer_len -data_len + index] != data[index]){
                return false;
            }
        }
    }
    return true;
}

static AtResponse wait_response(SoftwareSerial * _serial, uint32_t timeout){
    uint16_t start_time = millis();
    size_t response_index = 0;
    // Loop for waiting response
    while(true){
        // If timeout
        if(millis() - start_time > timeout){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.println("Timeout");
#endif
            return AT_GENERIC_ERROR;
        }
        // If response matched with at_response_table
        if(_serial->available() > 0){
            // // Append to reponse
            AT_BUFFER_RESPONSE[response_index++] = _serial->read();
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.write(AT_BUFFER_RESPONSE[response_index-1]);
#endif
            // If it oversize 
            if(response_index > AT_BUFFER_MAX_SIZE){
                return AT_GENERIC_ERROR;
            }
            for (size_t i = 0; i < AT_RESPONSE_SIZE; i++)
            {
                if(find(AT_BUFFER_RESPONSE,response_index, AT_RESPONSE_TABLE[i])){
                    return (AtResponse)i;
                }
            }
            
            
        }
    }
    return AT_GENERIC_ERROR;
}

static AtResponse wait_response_persistent(SoftwareSerial * _serial){
    static size_t response_index = 0;
    // Loop for waiting response
    if(_serial->available() > 0){
        // // Append to reponse
        AT_BUFFER_RESPONSE[response_index++] = _serial->read();
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
        Serial.write(AT_BUFFER_RESPONSE[response_index-1]);
#endif
        // If it oversize 
        if(response_index > AT_BUFFER_MAX_SIZE){
            response_index = 0;
            return AT_MAX;
        }
        for (size_t i = 0; i < AT_RESPONSE_SIZE; i++)
        {
            if(find(AT_BUFFER_RESPONSE,response_index, AT_RESPONSE_TABLE[i])){
                // Clear buffer
                memset(AT_BUFFER_RESPONSE, 0 , AT_BUFFER_MAX_SIZE);
                response_index = 0;
                return (AtResponse)i;
            }
        }
    }
    return AT_MAX;
}

static bool wait_response(SoftwareSerial * _serial , AtResponse expected, uint32_t timeout){
    const unsigned long start_time = millis();
    size_t response_index = 0;
    // Loop for waiting response
    while(true){
        // If timeout
        if(millis() - start_time > timeout){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.println("Timeout");
#endif
            return false;
        }
        // If response matched with at_response_table
        if(_serial->available() > 0){
            // // Append to reponse
            AT_BUFFER_RESPONSE[response_index++] = _serial->read();
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.write(AT_BUFFER_RESPONSE[response_index-1]);
#endif
            // If it oversize 
            if(response_index > AT_BUFFER_MAX_SIZE){
                return false;
            }
            if(find(AT_BUFFER_RESPONSE,response_index, AT_RESPONSE_TABLE[expected])){
                return true;
            }
        }
    }
    return false;
}

static void flush(SoftwareSerial * _serial, uint32_t timeout_ms){
    uint32_t start_time = millis();
    while(millis() - start_time < timeout_ms){
        if(_serial->available()){
            _serial->read();
        }
    }
}

BluetoothServer::BluetoothServer(){

}


BluetoothServer::BluetoothServer(SoftwareSerial * serial){
    _serial = serial;
}


bool BluetoothServer::restart(){
    size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+RST\r\n");
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }

    // Wait for Module Ready
    if(!wait_response(_serial, AT_SYSTEM_STARTUP)){
        return false;
    }

    return true;
}   

bool BluetoothServer::init(const char * device_name = "ESP32"){
    // Start Bluetooth Server Mode

    size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEINIT=2\r\n");
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Publish OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }

    // Set Device name

    len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLENAME=\"%s\"\r\n", device_name);
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Publish OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }

    // Flash all pending data in 10 ms
    flush(_serial, 10);

    return true;
}

bool BluetoothServer::start_uart_service(){

    // Create GATT Service
    size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTSSRVCRE\r\n");
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Get OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }


    // Start GATT Service
    len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTSSRVSTART\r\n");
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Get OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }

    // Start Advertising
    len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEADVSTART\r\n");
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Get OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }
    return true;
}

bool BluetoothServer::send_data(const char * data){
    send_data(data, strlen(data));
}

bool BluetoothServer::send_data(uint8_t * data, size_t data_len){
    if(!notify_is_enabled){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
        Serial.println("Client need to enable Notification");
        return false;
#endif        
    }
    // Notify data to Bluetooth Client
    size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTSNTFY=0,%d,%d,%d\r\n", uart_service_index, uart_char_tx_index, data_len);
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Get OK
    if(!wait_response(_serial, AT_GENERIC_INPUT)){
        return false;
    }

    // Send data
    _serial->write(data, data_len);
    
    // Wait for Get OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
        return false;
    }

    return true;
}

char* BluetoothServer::get_receive_data(){
    return rx_buffer;
}

bool BluetoothServer::set_on_receive_callback(void(*on_receive_callback)()){
    rx_callback = on_receive_callback;
}

void BluetoothServer::event_loop(){
    // TODO
    AtResponse response = wait_response_persistent(_serial);
    switch (response)
    {
    case AT_BLE_NOTIFY_ENABLED:
        if(!preprocess_notify_event(&notify_is_enabled)){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.println("Process notify event failed");
#endif
        }
        if(notify_is_enabled){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.println("Notify is enabled");
#endif         
        }else{
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.println("Notify is disabled");
#endif 
        }
        break;
    case AT_BLE_RX:
        memset(rx_buffer,0, sizeof(rx_buffer));
        if(!preprocess_write_event(rx_buffer, &rx_buffer_len)){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
        Serial.println("Process write event failed");
#endif
        }else{
            // Call rx callback
            rx_callback();
        }
        break;
    default:
        break;
    }
}

// Internal functions
bool BluetoothServer::preprocess_notify_event(bool *notify){
    uint32_t start_time = millis();
    uint16_t notify_data;
    uint8_t len = 0;
    // Clean before data
    uint8_t temp;
    while(1){
        if(millis() - start_time > at_timeout){
            return false;
        }
        if(_serial->available()){
            temp = _serial->read();
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.write(temp);
#endif
            // Check is stop recording
            if(temp == '\r'){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.println((uint16_t)notify_data);
#endif
                *notify =  (notify_data == 256) ? true : false;
                return true;
            }
            notify_data = notify_data << 8 | temp;
        }
    }
    return true;
}
bool BluetoothServer::preprocess_write_event(uint8_t *data, size_t * data_len){
    size_t real_len = 0;
    size_t _data_len = 0;
    bool start_record_len = true;
    bool start_record_data = false;
    uint32_t start_time = millis();
    uint16_t notify_data;
    uint8_t len = 0;
    // Clean before data
    uint8_t temp;
    while(1){
        if(millis() - start_time > at_timeout){
            return false;
        }
        if(_serial->available()){
            temp = _serial->read();
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
            Serial.write(temp);
#endif
            // Record datalen
            if(start_record_len){
                if(temp == ','){
                    start_record_len = false;
                    start_record_data = true;
                }else{
                    //  Call datalen
                    _data_len = _data_len * 10 + temp -48;
                }
            }
            // Record data
            else if(start_record_data){
                // Check is stop recording
                if(temp == '\r'){
                    if(_data_len != real_len){
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
                    Serial.println("Datalen and Real len is not equal");
#endif       
                        return false;
                    }
                    *data_len = _data_len;
                    return true;
                }else{
                    data[real_len++] = temp;
                }
            }
        }
    }
    return true;
}