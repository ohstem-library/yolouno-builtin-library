# Esp32 mqtt client library for YoloUno and Arduino Uno
This library allow user setup ESP32 connect to Wifi AP and using Mqtt API.

## Required
- ESP32 must be installed [this firmware](https://github.com/ohstem-library/esp32-mqtt)
- SoftwareSerial

## How to use it
### Wifi
- Restart Module
    ```
    bool Esp32::Wifi::restart()
    ```
- Connect to Wifi AP
    ```
    void Esp32::Wifi::connect_to_ap(String ssid, String password, int auth = 3)
    ```
### MQTT
- Connect to Mqtt Broker
    ```
    void Esp32::Mqtt::connect_mqtt_broker(String host,
                            int port,
                            const char* username = NULL,
                            const char* password = NULL,
                            int keepalive = 120,
                            int cleansession = 1,
                            const char* lw_topic = NULL,
                            const char* lw_payload = NULL)
    ```
- Subcribe to Topic
    ```
    bool Esp32::Mqtt::subcribe_topic(const char* topic, int qos = 1)
    ```
- Publish message
    ```
    bool Esp32::Mqtt::publish_message(const char* topic, const char* payload, int qos = 1, int retain = 0)
    ```
- On message callback
    ```
    void Esp32::on_message(String topic, String payload, int qos, int retain)
    ```
- Request mqtt client status
    ```
    bool Esp32::is_connected();
    ```
### HTTP
- Request: Currently only *GET* supported
    ```
    bool Esp32::Http::request(Esp32::Http::GET, "http://103.170.122.203/time.php")
    ```