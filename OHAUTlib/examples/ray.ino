#include "LEDDimmers.h"
#include <OHAUTlib.h>

#define DEVICE_TYPE "3CHANLED"

OHAUTservice ohaut;

void setupDimmers(ConfigMap *config_map) {
    float boot_values[3];
    for (int i=0;i<3; i++)
        boot_values[i] = getDimmerStartupVal(config_map, i);
    /* switch on leds */
    dimmers.setup(boot_values);
}

void setup(void){

    /* start the serial port and switch on the PCB led */
    Serial.begin(115200);
    pinMode(led_pin, OUTPUT);
    digitalWrite(led_pin, LOW);

    ohaut.on_config_loaded = &setupDimmers;
    ohaut.on_http_server_ready = &setupHTTPApi;
    ohaut.on_wifi_connected = &setupMQTTHandling;

    ohaut.setup(DEVICE_TYPE);

}

void loop(void){
    ohaut.handle();
}
