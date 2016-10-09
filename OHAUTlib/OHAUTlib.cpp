#include "OHAUTlib.h"

void configServerSetup(ESP8266WebServer *server, CONFIG_CALLBACK(cfg_callback));
bool wifiSetup();
void SSDP_setup(ESP8266WebServer *server, const char* name, const char *product);

OHAUTservice::OHAUTservice() {

    // initialize all the callbacks
    on_config_defaults = NULL;
    on_config_loaded = NULL;
    on_wifi_connected = NULL;
    on_http_server_ready = NULL;
    on_ota_start = NULL;
    on_ota_error = NULL;
    on_ota_end = NULL;

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

void OHAUTservice::setup(const char *device_type, const char* firmware_version,
                         const char *device_name) {

  _device_type = device_type;
  _firmware_version = firmware_version;
  _device_name = device_name;

  if (_led_pin>=0) {
    pinMode(_led_pin, OUTPUT);
    digitalWrite(_led_pin, LOW);
  }

  /* read the configuration, and setup the HTTP config server */
  configServerSetup(_server, this->on_config_defaults);

  if (on_config_loaded) 
    on_config_loaded(&configData);

  /* setup the /update-app/ server for app.html.gz updating */
  _upd_server->setup(_server, this);

  if (on_http_server_ready)
      on_http_server_ready(_server);

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

#if 1
  /* failsafe recovery during devel */
  for (int i=0;i<30; i++) {
      ArduinoOTA.handle();
      delay(100);
  }
#endif

  SSDP_setup(_server, configData["mqtt_id"], _device_name);
  _server->begin();
}

void OHAUTservice::handle() {
    ArduinoOTA.handle();
    _server->handleClient();
}

const char* OHAUTservice::get_firmware_version() {
   return _firmware_version;
}

const char* OHAUTservice::get_device_name() {
   return _device_name;
}

const char* OHAUTservice::get_device_type() {
    return _device_type;
}
