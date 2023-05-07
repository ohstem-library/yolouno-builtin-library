#include "Arduino.h"
#include "bluetooth.h"

typedef enum AtResponse{
    AT_GENERIC_OK = 0,
    AT_GENERIC_ERROR,
    AT_GENERIC_INPUT,
    // System Response
    AT_SYSTEM_STARTUP,
    // Bluetooth Response
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
    // [AT_BLE_SCAN_RESPONSE] = "+BLESCAN:",
    // [AT_BLE_SCAN_DONE_RESPONSE] = "+BLESCANDONE",
    // [AT_BLE_CONNECT_RESPONSE] = "+BLECONN:",
    // [AT_BLE_GATT_SERVICE_RESPONSE] = "+BLEGATTCPRIMSRV:",
    // [AT_BLE_GATT_CHARACTERISTIC_RESPONSE] = "+BLEGATTCCHAR:\"char\",",
    // [AT_BLE_GATT_CHARACTERISTIC_DESC_RESPONSE] = "+BLEGATTCCHAR:\"desc\","
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
    // Notify data to Bluetooth Client
    size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTSNTFY=0,%d,%d,<length>\r\n");
#if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
    Serial.print(AT_BUFFER);
#endif
    // Send At command
    _serial->write(AT_BUFFER, len);
    
    // Wait for Get OK
    if(!wait_response(_serial, AT_GENERIC_OK)){
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
    callback = on_receive_callback;
}

void BluetoothServer::event_loop(){
    // TODO
}