#include <Ticker.h>
#include <OHAUTlib.h>
#include "LEDDimmers.h"
#include "version.h"

#define DEVICE_TYPE "3CHANLED"

OHAUTservice ohaut;
LEDDimmers dimmers;
int led_pin = 13;

void setup(void){

    /* start the serial port and switch on the PCB led */
    Serial.begin(115200);

    ohaut.set_led_pin(led_pin);

    ohaut.on_config_defaults = [](ConfigMap *config) {
        config->set("mode", "lamp");
        config->set("startup_val_l0", "0");
        config->set("startup_val_l1", "0");
        config->set("startup_val_l2", "0");
    };

    ohaut.on_config_loaded = [](ConfigMap *configData) {
        float boot_values[3];
        for (int led=0;led<3; led++)
            boot_values[led] = getDimmerStartupVal(configData, led);
         /* switch on leds */
         dimmers.setup(boot_values);
    };

    ohaut.on_http_server_ready = &setupHTTPApi;

    ohaut.on_ota_start = [](){
        dimmers.halt();
    };

    ohaut.on_ota_error = [](ota_error_t error) {
        dimmers.restart();
    };

    ohaut.on_ota_end =  [](){
        for (int i=0;i<30;i++){
            analogWrite(led_pin,(i*100) % 1001);
            delay(50);
        }
    };

    ohaut.setup(DEVICE_TYPE, VERSION, "ray");
}

void loop(void){
    ohaut.handle();
    if (ohaut.is_wifi_connected()) {
        
    }
       
}

float getDimmerStartupVal(ConfigMap *configData, int dimmer) {
    char key[16];
    const char *val;
    sprintf(key, "startup_val_l%d", dimmer);
    val = (*configData)[key];

    if (val && strlen(val)) return atoi(val)/100.0;
    else                    return 1.0;
}
