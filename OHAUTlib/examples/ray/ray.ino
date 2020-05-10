#include <Ticker.h>
#include <OHAUTlib.h>
#include "LEDDimmers.h"
#include "version.h"
#include <ESPAsyncTCP.h>
#include <Hash.h>

#define DEVICE_TYPE "3CHANLED"

OHAUTservice ohaut;
LEDDimmers dimmers;
int led_pin = 13;

char lamp_name1[128];
char lamp_name2[128];
char lamp_name3[128];
char lamp_nameall[128];
float boot_values[3];

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

    // Add virtual devices
    sprintf(lamp_name1, "%s_l1", ohaut.get_host_id());
    sprintf(lamp_name2, "%s_l2", ohaut.get_host_id());
    sprintf(lamp_name3, "%s_l3", ohaut.get_host_id());
    sprintf(lamp_nameall, "%s_all", ohaut.get_host_id());

    ohaut.fauxmo->addDevice(lamp_name1);
    ohaut.fauxmo->addDevice(lamp_name2);
    ohaut.fauxmo->addDevice(lamp_name3);
    ohaut.fauxmo->addDevice(lamp_nameall);

    ohaut.fauxmo->onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {

      
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\r\n", device_id, device_name, state ? "ON" : "OFF", value);

        // Checking for device_id is simpler if you are certain about the order they are loaded and it does not change.
        // Otherwise comparing the device_name is safer.
        float valf;

        valf = state ? ((float)value/254.0) : 0.0;
        if (valf>1.0) {
            valf = 1.0;
        }

        if (strcmp(device_name, lamp_name1)==0) {
            dimmers.setDimmer(0, valf);
        } else if (strcmp(device_name, lamp_name2)==0) {
            dimmers.setDimmer(1, valf);
        } else if (strcmp(device_name, lamp_name3)==0) {
            dimmers.setDimmer(2, valf);
        } else if (strcmp(device_name, lamp_nameall)==0) {

            float boot_values[3];
            for (int led=0;led<3; led++)
                dimmers.setDimmer(led, valf * boot_values[led]);
        } 
    });
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
