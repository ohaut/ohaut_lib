#include "OHAUTlib.h"

void configServerSetup(WebServer *server, CONFIG_CALLBACK(cfg_callback));

OHAUTservice::OHAUTservice() {

    // initialize all the callbacks
    on_config_defaults = NULL;
    on_config_loaded = NULL;
    on_wifi_connected = NULL;
    on_http_server_ready = NULL;
    on_ota_start = NULL;
    on_ota_error = NULL;
    on_ota_end = NULL;
    on_ohaut_details = NULL;

    _led_pin = -1;
    _wifi_connected = false;
    _device_type = NULL;
    _device_name = NULL;
    _server = new WebServer(8080);
    _upd_server = new HTTPUpdateServer();
    _wifi_connection = new WifiConnection();
    fauxmo = new fauxmoESP();
}

OHAUTservice::~OHAUTservice() {
    delete _server;
    delete _upd_server;
    delete fauxmo;
    delete _wifi_connection;
}

void OHAUTservice::set_led_pin(int led) {
    _led_pin = led;
}

bool OHAUTservice::is_wifi_connected() {
    return _wifi_connection->connected();
}

void OHAUTservice::setup(const char *device_type, const char* firmware_version,
                         const char *device_name) {

  _device_type = device_type;
  _firmware_version = firmware_version;
  _device_name = device_name;

  Serial.println("");
  Serial.println("OHAUTservice: starting setup");

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

  _wifi_connection->setup();

  /* configure the Over The Air firmware upgrades */
  ArduinoOTA.setHostname(configData["host_id"]);
  if (on_ota_start) ArduinoOTA.onStart(on_ota_start);
  if (on_ota_error) ArduinoOTA.onError(on_ota_error);
  ArduinoOTA.onEnd([this](){
                    #ifdef ESP8266
                     if (_led_pin>=0) {
                         for (int i=0;i<30;i++){
                            analogWrite(_led_pin,(i*100) % 1001);
                            delay(50);
                         }
                     }
                     if (on_ota_end) on_ota_end();
                     #endif
                   });

  ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
    static bool toggle = true;
    toggle = !toggle;
    if (_led_pin) digitalWrite(_led_pin, toggle);
  });
  ArduinoOTA.begin();

  _server->begin();

  fauxmo->createServer(true);
  fauxmo->setPort(80);
  fauxmo->enable(true);

}

void OHAUTservice::handle() {
    _wifi_connection->handle();
    ArduinoOTA.handle();
    _server->handleClient();
    fauxmo->handle();
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

char* OHAUTservice::get_host_id() {
    return configData["host_id"];
}