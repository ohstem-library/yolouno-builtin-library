#include "Arduino.h"
#include "bluetooth.h"

namespace Esp32 {
    typedef enum AtResponse{
        AT_GENERIC_OK = 0,
        AT_GENERIC_ERROR,
        AT_GENERIC_INPUT,
        // System Response
        AT_SYSTEM_STARTUP,
        // Bluetooth Response
        // AT_BLE_SCAN_RESPONSE,
        // AT_BLE_SCAN_DONE_RESPONSE,
        // AT_BLE_CONNECT_RESPONSE,
        // AT_BLE_GATT_SERVICE_RESPONSE,
        // AT_BLE_GATT_CHARACTERISTIC_RESPONSE,
        // AT_BLE_GATT_CHARACTERISTIC_DESC_RESPONSE,
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

    static void event_loop(SoftwareSerial * _serial){

    }


//     BluetoothClient::BluetoothClient(){
//     }   

//     BluetoothClient::BluetoothClient(SoftwareSerial *serial){
//         _serial = serial;
//     }


//     bool BluetoothClient::init(){
//         AtResponse response;

//         size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEINIT=1\r\n");
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//         Serial.print(AT_BUFFER);
// #endif
//         // Send At command
//         _serial->write(AT_BUFFER, len);
        
//         // Wait for Publish OK
//         if(!wait_response(_serial, AT_GENERIC_OK)){
//             return false;
//         }

//         // Flash all pending data in 10 ms
//         flush(_serial, 10);

//         return true;
//     }


    

//     // Scan and return the device by name or not
//     bool BluetoothClient::scan(const char * device_name = NULL){
//         // Reset scan len
//         scan_index = 0;
//         //  Scan bleuetooth server
//         AtResponse response;

//         size_t len;
//         if(device_name != NULL){
//             len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLESCAN=1,0,2,\"%s\"\r\n", device_name);
//         }else{
//             len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLESCAN=1\r\n");
//         }
        
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//         Serial.print(AT_BUFFER);
// #endif
//         // Send At command
//         _serial->write(AT_BUFFER, len);
        
//         uint32_t start_time = millis();

//         while(1) {
//             if(millis() - start_time > scan_timeout){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 if(Serial){
//                     Serial.println("Bluetooth client scan done");
//                 }
// #endif
//                 break;
//             }
//             // Wait for SCAN response
//             if(wait_response(_serial, AT_BLE_SCAN_RESPONSE)){
//                 if(preprocess_scan_response()){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                     if(Serial){
//                         Serial.print("Scan found device: ");
//                         Serial.println(scan_list[scan_index-1]);
//                     }
// #endif
//                 }
//             }
//         }
//         // Disable scanning
//         len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLESCAN=0\r\n");
//         // Send At command
//         _serial->write(AT_BUFFER, len);

//         if(!wait_response(_serial, AT_BLE_SCAN_DONE_RESPONSE)){
//             return false;
//         }

//         // Flash all pending data in 50 ms
//         flush(_serial, 50);

//         if(scan_index == 0){
//             return false;
//         }
//         return true;
//     }

//     uint8_t BluetoothClient::get_scan_device_len(){
//         return scan_index;
//     }
    
//     char * BluetoothClient::get_scan_device_addr(uint8_t index){
//         return scan_list[index];
//     }

//     // Connect to BLuetooth server and check Nordic UART Service
//     bool BluetoothClient::connect(const char * device_name){
//         // Reset scan len
//         scan_index = 0;
//         //  Scan bleuetooth server
//         AtResponse response;

//         size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLECONN=0,\"%s\"\r\n", device_name);
        
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//         Serial.print(AT_BUFFER);
// #endif
//         // Send At command
//         _serial->write(AT_BUFFER, len);
        
//         if(!wait_response(_serial, AT_BLE_CONNECT_RESPONSE)){
//             return false;
//         }

//         if(!wait_response(_serial, AT_GENERIC_OK)){
//             return false;
//         }   
//         return true;
//     }


//     bool BluetoothClient::check_uart_service(){
//         gatt_service_index = 0;
//         memset(gatt_service_list,0, sizeof(gatt_service_list));
//         //  Discovery Gatt Service in Bluetooth Server
//         AtResponse response;

//         size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTCPRIMSRV=0\r\n");
        
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//         Serial.print(AT_BUFFER);
// #endif
//         // Send At command
//         _serial->write(AT_BUFFER, len);
        
//         uint16_t start_time = millis();

//         while(1) {
//             if(millis() - start_time > scan_timeout){
//                 break;
//             }
//             // Wait for SCAN response
//             if(wait_response(_serial, AT_BLE_GATT_SERVICE_RESPONSE)){
//                 if(preprocess_gatt_service_response()){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                     if(Serial){
//                         Serial.print("Gatt Service found: ");
//                         Serial.println(gatt_service_list[gatt_service_index]);
//                     }
// #endif
//                 }
//             }
//         }  
//         for (size_t i = 0; i <= gatt_service_index; i++)
//         {
//             if(strstr(gatt_service_list[i] , GATT_UART_SERVICE)){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.print("Bluetooth Client found device support UART service at index: ");
//                 Serial.println(i);
// #endif
//                 gatt_uart_service_index = i;
//             }
//         }

//         // Check Characteristic index
//         gatt_characteristic_index = 0;
//         memset(gatt_characteristic_list,0, sizeof(gatt_characteristic_list));

//         len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTCCHAR=0,%d\r\n", gatt_uart_service_index);
        
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//         Serial.print(AT_BUFFER);
// #endif
//         // Send At command
//         _serial->write(AT_BUFFER, len);
        
//         start_time = millis();

//         while(1) {
//             if(millis() - start_time > scan_timeout){
//                 break;
//             }
//             // Wait for SCAN response
//             uint8_t expected[2] = {AT_BLE_GATT_CHARACTERISTIC_RESPONSE, AT_BLE_GATT_CHARACTERISTIC_DESC_RESPONSE};
//             AtResponse response = wait_response(_serial);
//             if(response = AT_BLE_GATT_CHARACTERISTIC_RESPONSE){
//                 preprocess_gatt_characteristic_response();
//             }else if(response = AT_BLE_GATT_CHARACTERISTIC_DESC_RESPONSE){
//                 preprocess_gatt_characteristic_desc_response();
//             }
//         }  
        
//         bool found_rx = false;
//         bool found_tx = false;
//         for (size_t i = 0; i <= gatt_characteristic_index; i++)
//         {
//             if(strstr(gatt_characteristic_list[i] , GATT_UART_CHAR_TX)){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.print("Bluetooth Client found characteristic TX at index: ");
//                 Serial.println(i);
// #endif
//                 gatt_uart_tx_char_index = i;
//                 found_tx = true;
//             }
//             if(strstr(gatt_characteristic_list[i] , GATT_UART_CHAR_RX)){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.print("Bluetooth Client found characteristic RX at index: ");
//                 Serial.println(i);
// #endif
//                 gatt_uart_rx_char_index = i;
//                 found_rx = true;
//             }
//         }

//         // Enable Uart Rx service 
//         if(found_rx && found_tx){
//             return true;
//         }
//         return false;
//     }

//     // Send Data to Tx characteristic of BLE server
//     bool BluetoothClient::send_data(uint8_t * data, size_t data_len){
//         AtResponse response;

//         size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+BLEGATTCWR=0,%d,%d,%d\r\n", gatt_uart_service_index, gatt_uart_tx_char_index, data_len);
        
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//         Serial.print(AT_BUFFER);
// #endif
//         // Send At command
//         _serial->write(AT_BUFFER, len);
        
//         if(!wait_response(_serial, AT_GENERIC_INPUT)){
//             return false;
//         }

//         // Send At command
//         _serial->write(data, data_len);

//         if(!wait_response(_serial, AT_GENERIC_OK)){
//             return false;
//         }   
//         return true;
//     }

//     // Rx Data is available
//     bool BluetoothClient::receive_data_available(){

//     }

//     // Receive 1 byte from Rx buffer
//     uint8_t BluetoothClient::receive_data(){

//     }

//     bool BluetoothClient::preprocess_scan_response(){
//         bool start_recording = false;
//         uint32_t start_time = millis();
//         uint8_t len = 0;
//         // Clean before data
//         memset(scan_list[scan_index], 0 , sizeof(scan_list[scan_index]));
//         uint8_t temp;
//         while(1){
//             if(millis() - start_time > process_timeout){
//                 return false;
//             }
//             if(_serial->available()){
//                 temp = _serial->read();
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.write(temp);
// #endif
//                 if(!start_recording){
//                     if(temp == '"'){
//                         start_recording = true;
//                     }
//                 }
//                 else{
//                      // Check is stop recording
//                     if(temp == '"'){
//                         bool exist = false;
//                         //  Check device is exit in scan list
//                         for( uint8_t i = 0; i < scan_index ; i++){
//                             if(strstr(scan_list[i],scan_list[scan_index])){
//                                 if(Serial){
//                                     exist = true;
//                                 }
//                             }
//                         }
//                         // If not exist then add to list
//                         if(!exist){
//                             scan_index++;
//                             return true;
//                         }
//                         return false;
//                     }
//                     scan_list[scan_index][len++] = temp;
//                 }
               
//             }
//         }
//         return true;
//     }

//     bool BluetoothClient::preprocess_gatt_service_response(){
//         bool start_recording_index = false;
//         bool start_recording_service = false;
//         uint8_t service_index_found;
//         uint8_t split_count = 0;
//         uint32_t start_time = millis();
//         uint8_t len = 0;
//         // Clean before data
//         uint8_t temp;
//         while(1){
//             if(millis() - start_time > process_timeout){
//                 return false;
//             }
//             if(_serial->available()){
//                 temp = _serial->read();
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.write(temp);
// #endif
//                 if(!start_recording_index){
//                     if(temp == ','){
//                         start_recording_index = true;
//                     }
//                 }
//                 else if(!start_recording_service){
//                      // Check is stop recording
//                     if(temp == ','){
//                         start_recording_service = true;
//                     }else{
//                         service_index_found = temp - '0';
//                     }
                    
//                 }
//                 else{
//                      // Check is stop recording
//                     if(temp == ','){
//                         bool exist = false;
//                         //  Check device is exit in scan list
//                         for( uint8_t i = 0; i <= gatt_service_index ; i++){
//                             if(strstr(gatt_service_list[i],gatt_service_list[service_index_found])){
//                                 if(Serial){
//                                     exist = true;
//                                 }
//                             }
//                         }
//                         // If not exist then add to list
//                         if(!exist){
//                             gatt_service_index = service_index_found;
//                             return true;
//                         }
//                         return false;
//                     }
//                     gatt_service_list[service_index_found][len++] = temp;
//                 }
               
//             }
//         }
//         return true;
//     }

//     bool BluetoothClient::preprocess_gatt_characteristic_response(){
//         bool start_recording_index = false;
//         bool start_recording_characteristic = false;
//         uint8_t characteristic_index_found;
//         uint8_t split_count = 0;
//         uint32_t start_time = millis();
//         uint8_t len = 0;
//         // Clean before data
//         uint8_t temp;
//         while(1){
//             if(millis() - start_time > process_timeout){
//                 return false;
//             }
//             if(_serial->available()){
//                 temp = _serial->read();
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.write(temp);
// #endif
//                 if(!start_recording_index){
//                     if(temp == ','){
//                         split_count++;
//                         if(split_count == 2){
//                             start_recording_index = true;
//                         }
//                     }
//                 }
//                 else if(!start_recording_characteristic){
//                      // Check is stop recording
//                     if(temp == ','){
//                         start_recording_characteristic = true;
//                     }else{
//                         characteristic_index_found = temp - '0';
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                         Serial.print("Characteristic_index_found: ");
//                         Serial.println(characteristic_index_found);
// #endif
//                     }
//                 }
//                 else{
//                      // Check is stop recording
//                     if(temp == ','){
//                         bool exist = false;
//                         //  Check device is exit in scan list
//                         for( uint8_t i = 0; i <= gatt_characteristic_index ; i++){
//                             if(strstr(gatt_characteristic_list[i],gatt_characteristic_list[characteristic_index_found])){
//                                 if(Serial){
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                                     Serial.print("Characteristic: ");
//                                     Serial.println(gatt_characteristic_list[characteristic_index_found]);
// #endif
//                                     exist = true;
//                                 }
//                             }
//                         }
//                         // If not exist then add to list
//                         if(!exist){
//                             gatt_characteristic_index = characteristic_index_found;
//                             return true;
//                         }
//                         return false;
//                     }
//                     gatt_characteristic_list[characteristic_index_found][len++] = temp;
//                 }
//             }
//         }
//         return true;
//     }

//     bool BluetoothClient::preprocess_gatt_characteristic_desc_response(){
//         bool start_recording_index = false;
//         bool start_recording_characteristic = false;
//         uint8_t characteristic_index_found;
//         uint8_t split_count = 0;
//         uint32_t start_time = millis();
//         uint8_t len = 0;
//         // Clean before data
//         uint8_t temp;
//         while(1){
//             if(millis() - start_time > process_timeout){
//                 return false;
//             }
//             if(_serial->available()){
//                 temp = _serial->read();
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                 Serial.write(temp);
// #endif
//                 if(!start_recording_index){
//                     if(temp == ','){
//                         split_count++;
//                         if(split_count == 2){
//                             start_recording_index = true;
//                         }
//                     }
//                 }
//                 else if(!start_recording_characteristic){
//                      // Check is stop recording
//                     if(temp == ','){
//                         start_recording_characteristic = true;
//                     }else{
//                         characteristic_index_found = temp - '0';
//                     }
//                 }
//                 else{
//                      // Check is stop recording
//                     if(temp == ','){
//                         bool exist = false;
// #if defined(BLUETOOTH_DEBUG) && BLUETOOTH_DEBUG == 1
//                         Serial.print("Characteristic Desc index: ");
//                         Serial.println(gatt_characteristic_desc_index);
// #endif
//                         //  Check device is exit in scan list
//                         for( uint8_t i = 0; i <= gatt_characteristic_desc_index ; i++){
//                             if(strstr(gatt_characteristic_desc_list[i],gatt_characteristic_desc_list[characteristic_index_found])){
//                                 if(Serial){
//                                     exist = true;
//                                 }
//                             }
//                         }
//                         // If not exist then add to list
//                         if(!exist){
//                             gatt_characteristic_desc_index = characteristic_index_found;
//                             return true;
//                         }
//                         return false;
//                     }
//                     gatt_characteristic_desc_list[characteristic_index_found][len++] = temp;
//                 }
//             }
//         }
//         return true;
//     }

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
}