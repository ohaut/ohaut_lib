#ifndef __OHAUT_LIB_H
#define __OHAUT_LIB_H
#include <WiFiClient.h>
//#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Ticker.h>
#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#define WebServer ESP8266WebServer
#endif


#ifdef ESP32
#include <WebServer.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#endif

#if defined(ESP8266)
#include <ESPAsyncTCP.h>
#include <Hash.h>
#elif defined(ESP32)
#include <AsyncTCP.h>
#endif


#include "HTTPUpdateServer.h"
#include "ConfigMap.h"
#include <fauxmoESP.h>


extern ConfigMap configData;

#define CONFIG_CALLBACK(callback)       void (*callback)(ConfigMap *configData)
#define VOID_CALLBACK(callback)         void (*callback)()
#define HTTPSERVER_CALLBACK(callback)   void (*callback)(WebServer *server)
#define OTA_ERROR_CALLBACK(callback)    void (*callback)(ota_error_t error)

class OHAUTservice {
  public:
    // Events that can be consumed by devices

    CONFIG_CALLBACK(on_config_defaults);
    /*      Receives: ConfigMap*
     *      This handler can be used to provice device specific default
     *      settings and will be called before recovering settings from
     *      flash, and before on_config_loaded is called
     */

    CONFIG_CALLBACK(on_config_loaded);
    /*      Receives: ConfigMap*
     *      This handler can be used to make the device act on specific
     *      settings once the configuration has been loaded from flash,
     *      for example to initialize hardware elements
     */

    VOID_CALLBACK(on_wifi_connected);
    /*      This handler is called once the device has wifi connectivity
     *      as an STA device (not when in Access Point mode)
     */

    HTTPSERVER_CALLBACK(on_http_server_ready);
    /*      Receives: ESP8266WebServer*
     *      This handler is called when the HTTP server is ready, so the
     *      device can add it's own HTTP API handlers.
     */

    VOID_CALLBACK(on_ota_start);
    /*      This handler is called when the Over The Air upgrade protocol
     *      is started. The device should probably disable any hardware that
     *      is real-time critical (like PWMs, etc), and has te oportunity
     *      to signal the startup of OTA.
     */

    OTA_ERROR_CALLBACK(on_ota_error);
    /*      Receives: ota_error_t
     *      This handler is called if the OTA upgrade failed, so the
     *      real-time critical hardware can be re-enabled.
     */

    VOID_CALLBACK(on_ota_end);
    /*      This handler is called if the OTA upgrade finished properly,
     *      so the device can do anything it may want to do right before
     *      reboot.
     */

    CONFIG_CALLBACK(on_ohaut_details);
    /*      Receives: ConfigMap*
     *      This handler is called so the device can add any key/value to the
     *      /ohaut/<device_id>/details json entry
     */

    bool is_wifi_connected();
    void set_led_pin(int led_pin);
    const char* get_device_name();
    const char* get_device_type();
    const char* get_firmware_version();
    char* get_host_id();

    void setup(const char *device_type,
               const char *firmware_version,
               const char *device_name);

    void handle();

    fauxmoESP*  fauxmo;

    OHAUTservice();
    ~OHAUTservice();

  private:
    WebServer* _server;
    HTTPUpdateServer* _upd_server;

    int _led_pin;
    bool _wifi_connected;
    bool _mqtt_enabled;
    const char* _device_type;
    const char* _device_name;
    const char* _firmware_version;
    void _setup_mqtt();
};
#endif
