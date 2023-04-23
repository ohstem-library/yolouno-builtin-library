#ifndef ESP32_H
#define ESP32_H
#include "Arduino.h"
#include "SoftwareSerial.h"

#define ESP32_DEBUG 1

namespace Esp32 {
    // On message callback table
    typedef struct {
        char * topic;
        void (*func)();
        char * payload;
    }on_message_cb_t;

    // AT response
    typedef enum AtResponse{
        AT_GENERIC_OK = 0,
        AT_GENERIC_ERROR,
        // System Response
        AT_SYSTEM_STARTUP,
        // Wifi Response
        AT_WIFI_CONNECTED,
        AT_WIFI_DISCONNECTED,
        AT_WIFI_GOT_IP,
        AT_SMARTCONFIG_CONNECTED,
        // Mqtt Reponse
        AT_MQTT_CONNECTED,
        AT_MQTT_DISCONNECTED,
        AT_MQTT_ON_MESSAGE,
        AT_MQTT_PUBLISH_OK,
        AT_MQTT_PUBLISH_FAIL,
        // Http Response
        AT_HTTPCLIENT_RESPONSE,
        AT_MAX
    };
    
    static const uint16_t AT_TIMEOUT = 10000; // 500ms
    static const uint16_t AT_BUFFER_MAX_SIZE = 256; // 2048 bytes
    static char AT_BUFFER[AT_BUFFER_MAX_SIZE] = {0};
    static char AT_BUFFER_RESPONSE[AT_BUFFER_MAX_SIZE] = {0};

    static const uint8_t AT_RESPONSE_SIZE = AT_MAX;

    static const char * AT_RESPONSE_TABLE[AT_MAX] = {
        [AT_GENERIC_OK] = "OK",
        [AT_GENERIC_ERROR] = "ERROR",
        // System Response
        [AT_SYSTEM_STARTUP] = "ready",
        // Wifi Reponse
        [AT_WIFI_CONNECTED] = "WIFI CONNECTED",
        [AT_WIFI_DISCONNECTED] = "WIFI DISCONNECT",
        [AT_WIFI_GOT_IP] = "WIFI GOT IP",
        [AT_SMARTCONFIG_CONNECTED] = "smartconfig connected wifi",
        // Mqtt Response
        [AT_MQTT_CONNECTED] = "+MQTTCONNECTED",
        [AT_MQTT_DISCONNECTED] = "+MQTTDISCONNECTED",
        [AT_MQTT_ON_MESSAGE] = "+MQTTSUBRECV",
        [AT_MQTT_PUBLISH_OK] = "+MQTTPUB:OK",
        [AT_MQTT_PUBLISH_FAIL] = "+MQTTPUB:FAIL",
        [AT_HTTPCLIENT_RESPONSE] = "+HTTPCLIENT:"
    };
    static AtResponse persistent_response = AT_MAX;
    static const size_t CALLBACK_MAX_LEN = 10;
    static on_message_cb_t on_message_callback_table[CALLBACK_MAX_LEN] = {{0,0}};
    static size_t callback_index = 0;

    AtResponse wait_response(SoftwareSerial * _serial, uint32_t timeout = AT_TIMEOUT);
    AtResponse wait_response_persistent(SoftwareSerial * _serial);
    bool wait_response(SoftwareSerial * _serial, AtResponse expected_response, uint32_t timeout = AT_TIMEOUT);
    void flush(SoftwareSerial * _serial, uint32_t timeout_ms);
    // Event
    void event_loop(SoftwareSerial *_serial);
    // Callback handler
    void on_message_callback( char* topic,  char* payload);
    // On mqtt message callback
    bool preprocess_on_message(SoftwareSerial *_serial, char *topic, char * payload);

    class Wifi{
        private: 
            SoftwareSerial * _serial;

        public:
            Wifi();
            Wifi(SoftwareSerial *serial);
            ~Wifi();
            bool restart();
            bool connect_to_ap(const char* ssid, const char* password);
            bool enable_smart_config();
    };

    class Mqtt{
        private:
            // Serial
            SoftwareSerial * _serial;
            char *_username;
        public:
            Mqtt();
            Mqtt(SoftwareSerial * serial);
            ~Mqtt();
            bool connect_mqtt_broker(
                                        char* host,
                                        int port,
                                        char* username = "",
                                        char* password = "",
                                        int keepalive = 120,
                                        int cleansession = 1,
                                        char* lw_topic = "",
                                        char* lw_payload = "");
            bool subcribe_topic(char* topic, void (*func)(), int qos = 1);
            bool publish_message(const char* topic, const char* payload, int qos = 1, int retain = 0);
            bool is_connected();
            char* getMessageFrom(const char * topic);
    };
    
    class Http{
        private:
            // Serial
            SoftwareSerial * _serial;
            String data;
            bool process_get_message();
        public:
            typedef enum RequestType {
                HEAD = 1,
                GET = 2,
                POST = 3,
                PUT = 4,
                DELETE = 5
            };

            typedef enum ContentType {
                WWW_FORM = 0,
                APP_JSON = 1,
                FORM_DATA = 2,
                TEXT_XML = 3
            };

            typedef enum TransportType {
                TCP = 1,
                SSL
            };
            Http();
            Http(SoftwareSerial *serial);
            ~Http();
            bool request(RequestType reqType , const char* url, uint32_t timeout = 10000, ContentType conType = WWW_FORM, TransportType tranType = TCP);
            String getDataResponse();
    };
}

#endif





