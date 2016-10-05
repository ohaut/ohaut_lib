#include "OHAUTlib.h"

void configServerSetup(ESP8266WebServer *server);
bool wifiSetup();
void SSDP_setup(ESP8266WebServer *server, const char* name, const char *product);

OHAUTservice::OHAUTservice() {
    on_config_loaded = NULL;
    on_wifi_connected = NULL;
    on_http_server_ready = NULL;

    _led_pin = -1;
    _wifi_connected = false;
    _device_type = NULL;
    _device_name = NULL;
    _server = new ESP8266WebServer();
    _upd_server = new HTTPUpdateServer();
}

OHAUTservice::~OHAUTservice() {
    delete _server;
    delete _upd_server;
}

void OHAUTservice::set_led_pin(int led) {
    _led_pin = led;
}

bool OHAUTservice::is_wifi_connected() {
    return _wifi_connected;
}

void OHAUTservice::setup(const char *device_type, const char *device_name) {

  _device_type = device_type;
  _device_name = device_name;

  if (_led_pin>=0) {
    pinMode(_led_pin, OUTPUT);
    digitalWrite(_led_pin, LOW);
  }

  /* read the configuration, and setup the HTTP config server */
  configServerSetup(_server);

  if (on_config_loaded) 
    on_config_loaded(&configData);

  /* setup the /update-app/ server for app.html.gz updating */
  _upd_server->setup(_server);

  /* try to connect to the wifi, otherwise we will have an access point */
  _wifi_connected = wifiSetup();

  /* configure the Over The Air firmware upgrades */
  ArduinoOTA.setHostname(configData["mqtt_id"]);
  if (on_ota_start) ArduinoOTA.onStart(on_ota_start);
  if (on_ota_error) ArduinoOTA.onError(on_ota_error);
  ArduinoOTA.onEnd([this](){
                     if (_led_pin>=0) {
                         for (int i=0;i<30;i++){
                            analogWrite(_led_pin,(i*100) % 1001);
                            delay(50);
                         }
                     }
                     if (on_ota_end) on_ota_end();
                   });

  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
    static bool toggle = true;
    toggle = !toggle;
    if (_led_pin) digitalWrite(_led_pin, toggle);
  });
  ArduinoOTA.begin();

  SSDP_setup(_server, configData["mqtt_id"], _device_name);
}

void OHAUTservice::handle() {
    ArduinoOTA.handle();
    _server->handleClient();
}
