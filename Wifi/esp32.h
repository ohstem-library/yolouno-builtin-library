#ifndef ESP32_H
#define ESP32_H
#include "Arduino.h"
#include "SoftwareSerial.h"

#define ESP32_DEBUG 1

namespace Esp32 {

    // Event
    void event_loop(SoftwareSerial *_serial);
    class Wifi{
        private: 
            SoftwareSerial * _serial;

        public:
            Wifi();
            Wifi(SoftwareSerial *serial);
            ~Wifi();
            bool restart();
            bool mode_sta();
            bool set_disconnected_callback(void(*_on_disconnect_callback)());
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
            bool publish_message(const char* topic, String payload, int qos = 1, int retain = 0);
            bool publish_message(const char* topic, int payload, int qos = 1, int retain = 0);
            bool publish_message(const char* topic, float payload, int qos = 1, int retain = 0);
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





