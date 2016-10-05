#ifndef __OHAUT_LIB_H
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#define MQTT_MAX_PACKET_SIZE 512
#include <PubSubClient.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266SSDP.h>
#include "HTTPUpdateServer.h"
#include "ConfigMap.h"

extern ConfigMap configData;

#define CONFIG_CALLBACK(callback)       void (*callback)(ConfigMap *configData)
#define VOID_CALLBACK(callback)         void (*callback)()
#define HTTPSERVER_CALLBACK(callback)   void (*callback)(ESP8266WebServer *server)
#define OTA_ERROR_CALLBACK(callback)    void (*callback)(ota_error_t error)

class OHAUTservice {
  public:
    CONFIG_CALLBACK(on_config_loaded);
    VOID_CALLBACK(on_wifi_connected);
    HTTPSERVER_CALLBACK(on_http_server_ready);
    VOID_CALLBACK(on_ota_start);
    OTA_ERROR_CALLBACK(on_ota_error);
    VOID_CALLBACK(on_ota_end);

    bool is_wifi_connected();
    void set_led_pin(int led_pin);

    void setup(const char *device_type, const char *device_name);

    void handle();

    OHAUTservice();
    ~OHAUTservice();

  private:
    ESP8266WebServer* _server;
    HTTPUpdateServer* _upd_server;
    int _led_pin;
    bool _wifi_connected;
    const char* _device_type;
    const char* _device_name;
};
#endif
