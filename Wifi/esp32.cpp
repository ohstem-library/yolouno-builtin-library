#include "esp32.h"
#include "string.h"

namespace Esp32{
    /* *****************************************Utils******************************************/
    bool find(char *buffer , uint16_t buffer_len , const char * data){
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

    AtResponse wait_response(SoftwareSerial * _serial, uint32_t timeout){
        const unsigned long start_time = millis();
        size_t response_index = 0;
        // Loop for waiting response
        while(true){
            // If timeout
            if(millis() - start_time > timeout){
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
                Serial.println("Timeout");
#endif
                return AT_GENERIC_ERROR;
            }
            // If response matched with at_response_table
            if(_serial->available() > 0){
                // // Append to reponse
                AT_BUFFER_RESPONSE[response_index++] = _serial->read();
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
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

    AtResponse wait_response_persistent(SoftwareSerial * _serial){
        static size_t response_index = 0;
        // Loop for waiting response
        if(_serial->available() > 0){
            // // Append to reponse
            AT_BUFFER_RESPONSE[response_index++] = _serial->read();
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
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

    bool wait_response(SoftwareSerial * _serial , AtResponse expected, uint32_t timeout){
        const unsigned long start_time = millis();
        size_t response_index = 0;
        // Loop for waiting response
        while(true){
            // If timeout
            if(millis() - start_time > timeout){
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
                Serial.println("Timeout");
#endif
                return false;
            }
            // If response matched with at_response_table
            if(_serial->available() > 0){
                // // Append to reponse
                AT_BUFFER_RESPONSE[response_index++] = _serial->read();
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
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

    void flush(SoftwareSerial * _serial, uint32_t timeout_ms){
        uint16_t start_time = millis();
        while(millis() - start_time < timeout_ms){
            if(_serial->available()){
                _serial->read();
            }
        }
    }

    void event_loop(SoftwareSerial * _serial){
        static char topic[64] = {0};
        static char payload[64] = {0};
        AtResponse response = wait_response_persistent(_serial);
        switch (response)
        {
        case AT_MQTT_ON_MESSAGE:
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
            Serial.println("AT_MQTT_ON_MESSAGE");
#endif
            memset(topic, 0 ,sizeof(topic));
            memset(payload, 0 ,sizeof(payload));
            if(preprocess_on_message(_serial, topic, payload)){
                on_message_callback(topic,payload);
            }
            break;
        default:
            break;
        }
    }

    bool preprocess_on_message(SoftwareSerial *_serial, char *topic, char * payload){
        uint16_t topic_len = 0;
        uint16_t payload_len = 0;
        uint16_t start_time = millis();
        uint8_t split_character_cnt = 0;
        char split_character = ',';
        bool success = true;
        while(true){
            if(millis() - start_time > 5000){
                success = false;
                break;
            }
            if(_serial->available() > 0){
                char temp = (char)_serial->read();
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
                Serial.print(temp);
#endif
                // If get endofline -> return
                if(temp == '\n'){
                    break;
                }
                if(temp == split_character){
                    split_character_cnt++;
                }else{
                    switch(split_character_cnt){
                        case 1:
                            topic[topic_len++] = temp;
                            break;
                        case 3:
                            payload[payload_len++] = temp;
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        for(int i = 0; i < topic_len - 1; i++){
            topic[i] = topic[i+1];
        }
        topic[topic_len - 1] = '\0';
        return success;
    }

    void on_message_callback( char* topic, char* payload){
        for(int i=0; i < callback_index; i++){
            if(strstr(topic, on_message_callback_table[i].topic)){
                on_message_callback_table[i].payload = payload;
                on_message_callback_table[i].func();
                return;
            }
        }
    }

    /* *****************************************Wifi******************************************/
    Wifi::Wifi(){
        _serial = NULL;
    }
    Wifi::Wifi(SoftwareSerial *serial){
        _serial = serial;
    }

    bool Wifi::restart(){
        if(wait_response(_serial, AT_SYSTEM_STARTUP, 5000)){  // Maximum 5 seconds
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.println("Wifi started");
#endif
            return true;
        }
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.println("restart");
#endif
        AtResponse response;
        // Atcommand
        size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+RST\n");

        // Send At command
        _serial->write(AT_BUFFER, len);

        if(!wait_response(_serial, AT_SYSTEM_STARTUP, 20000)){  // Maximum 20 seconds
            return false;
        }

        // Flush all pending data
        flush(_serial, 100);
        return true;
    }
    bool Wifi::connect_to_ap(const char* ssid, const char* password){
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.println("connect_to_ap");
#endif
        AtResponse response;
        // Atcommand
        size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+CWJAP=\"%s\",\"%s\"\n" , ssid, password);

        // Send At command
        _serial->write(AT_BUFFER, len);

        // Wait for Wifi Connected
        if(!wait_response(_serial, AT_WIFI_CONNECTED)){
            return false;
        }
        
        // Wait for Wifi Connected
        if(!wait_response(_serial, AT_WIFI_GOT_IP)){
            return false;
        }

        // Flash all pending data in 100 ms
        flush(_serial, 100);

        return true;
    }

    Wifi::~Wifi(){

    }

    bool Wifi::enable_smart_config(){

    }

    /* *****************************************Mqtt******************************************/
    Mqtt::Mqtt(){
        _serial = NULL;
    }
    
    Mqtt::Mqtt(SoftwareSerial *serial){
        _serial = serial;
    }

    Mqtt::~Mqtt(){

    }

    bool Mqtt::connect_mqtt_broker(
                                char* host,
                                int port,
                                char* username ,
                                char* password ,
                                int keepalive,
                                int cleansession ,
                                char* lw_topic,
                                char* lw_payload ){
        _username = username;
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.println(password);
#endif
        /****Client config***/ 
        // Atcommand
        size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+MQTTUSERCFG=0,1,\"c-%u\",\"%s\",\"%s\",0,0,\"\"\n", (uint16_t)millis(), username, password);
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.print(AT_BUFFER);
#endif
        // Send At command
        _serial->write(AT_BUFFER, len);

        // Wait for get AT_OK
        if(!wait_response(_serial, AT_GENERIC_OK)){
            return false;
        }
        // Flash all pending data in 100 ms
        flush(_serial, 50);

        // Atcommand
        len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+MQTTCONN=0,\"%s\",%d,1\n" ,host, port);
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.print(AT_BUFFER);
#endif
        // Send At command
        _serial->write(AT_BUFFER, len);

        // Wait for get AT_MQTT_CONNECTED
        if(!wait_response(_serial, AT_MQTT_CONNECTED)){
            return false;
        }

        // Flash all pending data in 100 ms
        flush(_serial, 50);

        return true;
    }

    bool Mqtt::subcribe_topic(char* topic, void (*func)(), int qos){
        AtResponse response;

        size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+MQTTSUB=0,\"/%s/feeds/%s\",%d\n", _username, topic, qos);
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.print(AT_BUFFER);
#endif
        // Send At command
        _serial->write(AT_BUFFER, len);

        // Wait for AT OK
        if(!wait_response(_serial, AT_GENERIC_OK)){
            return false;
        }

        // Register to callback function table
        if(callback_index >= CALLBACK_MAX_LEN){
            return false;
        }

        on_message_callback_table[callback_index].topic = topic;
        on_message_callback_table[callback_index].func = func;
        callback_index++;

        // Flash all pending data in 50 ms
        flush(_serial, 50);

        return true;
    }

    bool Mqtt::publish_message(const char* topic, const char* payload, int qos, int retain){
        AtResponse response;

        size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, "AT+MQTTPUB=0,\"/feeds/%s/%s\",\"%s\",%d,%d\n" ,_username, topic, payload, qos, retain );
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.print(AT_BUFFER);
#endif
        // Send At command
        _serial->write(AT_BUFFER, len);
        
        // Wait for Publish OK
        if(!wait_response(_serial, AT_GENERIC_OK)){
            return false;
        }

        // Flash all pending data in 50 ms
        flush(_serial, 50);

        return true;
    }

    bool Mqtt::is_connected(){

    }

    char* Mqtt::getMessageFrom(const char * topic){
        for (uint16_t i = 0; i < callback_index; i++)
        {
            if(strstr(on_message_callback_table[i].topic, topic)){
                return on_message_callback_table[i].payload;
            }
        }
        return "";
    }

    /******************************************Http******************************************/
    Http::Http(){
        _serial = NULL;
    }

    Http::Http(SoftwareSerial *serial){
        _serial = serial;
    }

    Http::~Http(){
        
    }

    bool Http::process_get_message(){
        data = "";
        uint16_t data_len = 0;
        uint16_t start_time = millis();
        bool start_record = false;
        char split_character = ',';
        uint16_t split_count = 0;
        while(true){
            if(millis() - start_time > 5000){
                return false;
            }
            if(_serial->available() > 0){
                char temp = (char)_serial->read();
                if(temp == split_character){
                    split_count ++;
                }else{
                    switch (split_count)
                    {
                    case 0:
                        data_len = data_len * 10 + (temp - '0');
                        break;
                    case 1:
                        data += String(temp);
                        data_len --;
                        if(data_len == 0){
                            return true;
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        return false;
    }
    
    bool Http::request(RequestType reqType , const char* url, uint32_t timeout, ContentType conType, TransportType tranType){
        AtResponse response;

        size_t len = snprintf(AT_BUFFER, AT_BUFFER_MAX_SIZE, 
                                    "AT+HTTPCLIENT=%d,%d,\"%s\",\"\",\"\",%d\n" ,reqType, conType, url, tranType);
#if defined(ESP32_DEBUG) && ESP32_DEBUG == 1
        Serial.print(AT_BUFFER);
#endif
        // Send At command
        _serial->write(AT_BUFFER, len);
        
        // Wait for Http Response
        if(!wait_response(_serial, AT_HTTPCLIENT_RESPONSE,timeout)){
            return false;
        }

        // Process Message
        process_get_message();

        return true;
    }

    String Http::getDataResponse(){
        return data;
    }
}




