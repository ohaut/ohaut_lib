#include <OHAUTlib.h>

#define DEVICE_TYPE "ray"
#define FIRMWARE_VERSION "000001"

OHAUTservice ohaut;

void setupDimmers(ConfigMap* map) {
}

void setupHTTPApi(WebServer* ws) {
}

void setupAfterWifi() {
}


void setup(void){

    Serial.begin(115200);

    ohaut.on_config_loaded = &setupDimmers;
    ohaut.on_http_server_ready = &setupHTTPApi;
    ohaut.on_wifi_connected = &setupAfterWifi;

    ohaut.setup(DEVICE_TYPE, FIRMWARE_VERSION, "ray");

    ohaut.fauxmo->addDevice("minimal_l1");
    ohaut.fauxmo->addDevice("minimal_l2");

    ohaut.fauxmo->onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\r\n", device_id, device_name, state ? "ON" : "OFF", value);
    });

}

void loop(void){
    ohaut.handle();
}
