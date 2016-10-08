#include <OHAUTlib.h>

#define DEVICE_TYPE "TESTDEV"
#define FIRMWARE_VERSION "000001"

OHAUTservice ohaut;

void setupDimmers(ConfigMap* map) {
}

void setupHTTPApi(ESP8266WebServer* ws) {
}

void setupMQTTHandling() {
}


void setup(void){

    Serial.begin(115200);

    ohaut.on_config_loaded = &setupDimmers;
    ohaut.on_http_server_ready = &setupHTTPApi;
    ohaut.on_wifi_connected = &setupMQTTHandling;

    ohaut.setup(DEVICE_TYPE, FIRMWARE_VERSION, "Test Device");

}

void loop(void){
    ohaut.handle();
}
