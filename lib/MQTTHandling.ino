#include "MQTTDevice.h"

MQTTDevice mqtt;
bool mqtt_enabled = false;

void setDimmerAndPublish(int channel, int value) {

    dimmers.setDimmer(channel, (float)value/100.0);
    if (mqtt_enabled) {
      char  pub_name[8];
      char  pub_data[6];

      sprintf(pub_name, "led%d", channel+1);
      sprintf(pub_data, "%d", value);
      mqtt.publish(pub_name, pub_data);
    }
}

bool startup_values_sent = false;

void dimmer_status(int led, byte* payload, unsigned int length) {
  char str[4];

  if (length>3)
    return;

  if (!startup_values_sent) return;

  memset(str, 0, length);
  memcpy(str, payload, length);

  String value(str);
  dimmers.setDimmer(led, value.toFloat()/100.0);

}


void sendMQTTStartupValues() {
  if (mqtt.connected() && !startup_values_sent)
  {
    for (int i=0; i<3; i++) {
      char cf_key[32], pub_name[8];
      const char *cf_data;
      sprintf(cf_key, "startup_val_l%d", i);
      sprintf(pub_name, "led%d", i+1);
      cf_data = configData[cf_key];
      if (cf_data && strlen(cf_data))
        mqtt.publish(pub_name, cf_data);
    }
    startup_values_sent = true;
  }
}


bool ohaut_integration_enabled() {
  return strcmp(configData["oh_int"], "1") == 0;
}

bool setupMQTTHandling(const char* device_type) {
  if (strlen(configData["mqtt_server"])>0) {
    mqtt.setup(configData["mqtt_server"],
               configData["mqtt_path"],
               configData["mqtt_id"]);


    if (ohaut_integration_enabled()) {
      mqtt.setupOhaut(device_type,
                      configData["oh_room"],
                      configData["oh_section"],
                      configData["oh_name"],
                      atoi(configData["oh_order"]));
    }

    mqtt.setHandler("led1", [](byte *data, unsigned int length) {
                                dimmer_status(0, data, length); });
    mqtt.setHandler("led2", [](byte *data, unsigned int length) {
                                dimmer_status(1, data, length); });
    mqtt.setHandler("led3", [](byte *data, unsigned int length) {
                                dimmer_status(2, data, length); });


    mqtt_enabled = true;
    return true;
  }
  mqtt_enabled = false;
  return false;

}

void MQTTHandle() {
  if (mqtt_enabled) {
    mqtt.handle();
    sendMQTTStartupValues();
  }
}
