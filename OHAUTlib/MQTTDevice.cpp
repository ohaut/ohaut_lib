#include <Arduino.h>
#include <WifiClient.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "MQTTDevice.h"
#include "OHAUTlib.h"
#include "ConfigMap.h"

int reconnect_retry_time = 1000;
const int max_reconnect_retry_time = 60000*2;

MQTTDevice::MQTTDevice(OHAUTservice* ohaut_service) {
    _client = new PubSubClient(_mqttClient);
    _ohaut = ohaut_service;
}

void MQTTDevice::setup(const char* server, const char *client_id,
                       const char* user, const char* pass){

  _server = server;
  _last_will_path = NULL;
  _last_will_val = NULL;
  _type = NULL;
  _room = NULL;
  _section = NULL;
  _order = -1;
  _last_will_qos = 0;
  _client_id = client_id;
  _user = user;
  _pass = pass;
  _elements = NULL;
  _last_reconnect = millis() - reconnect_retry_time;
  setup();
}

void MQTTDevice::setupOhaut(const char* type, const char* room,
                            const char* section, const char* friendly_name,
                            int order) {

  _type = type;
  _room = room;
  _section = section;
  _order = order;
  _friendly_name = friendly_name;

  sprintf(_device_id, "%s_%08x",
          _ohaut->get_device_name(),
          ESP.getChipId());

  String device_path = _getOhautPathFor("online");
  setLastWill(device_path.c_str(), "0", MQTTQOS2);

}

const char* MQTTDevice::_trueFalse(bool true_false) {
  return true_false ? "1": "0";
}

void MQTTDevice::publishOhautDetails() {

  ConfigMap desc;

  desc.set("version", "0");
  desc.set("room", _room);
  desc.set("section", _section);
  desc.set("order", String(_order, DEC));
  desc.set("has_rules", _trueFalse(SPIFFS.exists("/openhab/sitemap")));
  desc.set("has_sitemap", _trueFalse(SPIFFS.exists("/openhab/rules")));
  desc.set("ip", WiFi.localIP().toString());
  desc.set("type", _type);
  desc.set("name", _friendly_name);
  if (_ohaut->on_ohaut_details) 
      _ohaut->on_ohaut_details(&desc);
  publishOhautNode("details", desc.toJsonStr().c_str());
}

void MQTTDevice::setLastWill(const char *subpath, const char *value, int qos) {
  if (_last_will_path) delete _last_will_path;
  _last_will_path = strdup(subpath);
  _last_will_val = value;
  _last_will_qos = qos;
}

void MQTTDevice::setHandler(const char *name, SUBS_CALLBACK(fn)) {
  SubscribedElement** element=&_elements;

  while(*element!=NULL)
    element = &((*element)->next);

  *element = new SubscribedElement(name, fn);
}

void MQTTDevice::_publish(String path, const char *value) {
  _client->publish(path.c_str(), value, true /* retained */);
  for (int i=0; i<20; i++) {
    _client->loop();
    delay(5);
  }
}

void MQTTDevice::publish(const char *name, const char *value) {
  String path = _getPathFor(name);
  _publish(path, value);
  path += "/state";
  _publish(path, value);
}

void MQTTDevice::publishOhautNode(const char *node, const char *value) {
  String path = _getOhautPathFor(node);
  _publish(path, value);
}

MQTTDevice *_singleton = NULL;

void mqtt_handle_message(char* topic, byte* payload, unsigned int length) {
  if (_singleton)
    _singleton->_handle_message(topic, payload, length);
}

void MQTTDevice::setup(){
  _singleton = this;
  _client->setServer(_server, 1883);
  _client->setCallback(mqtt_handle_message);
}

String MQTTDevice::_getPathFor(const char *name) {
  String path = "element/";
  path += name;
  return _getOhautPathFor(path.c_str());
}

String MQTTDevice::_getOhautPathFor(const char *node) {
  String path = "/ohaut/";
  path += _device_id;
  path += "/";
  path += node;
  return path;
}

void MQTTDevice::_handle_message(char* topic, byte* payload, unsigned int length) {

  for (SubscribedElement *element=_elements; element!=NULL; element=element->next) {
    if (_getPathFor(element->name) == topic) {
      element->callback(payload, length);
    }
  }
}

void MQTTDevice::handle() {
  _client->loop();

  if (!_client->connected()) {
    long now = millis();
    if (now - _last_reconnect > reconnect_retry_time)
    {
      _reconnect();
      _last_reconnect = now;
    }
  }
}

bool MQTTDevice::connected() {
  return _client->connected();
}

int MQTTDevice::_connect() {
  Serial.printf("MQTTDevice: Attempting connection to %s ...\r\n", _server);
  if (_last_will_path) {
    // TODO(mangelajo): add authentication and SSL support
    return _client->connect(_client_id,
                            _last_will_path, _last_will_qos,
                            true, // willRetain
                            _last_will_val);
  } else {
    return _client->connect(_client_id);
  }
}

void MQTTDevice::_reconnect() {
  if (!_client->connected()) {
    // Attempt to connect
    if (_connect()) {
        Serial.println("MQTTDevice: connected!");
        // wildcard subscription to everything under our elements path
        _client->subscribe(_getPathFor("element/#").c_str());
        for (int i=0; i<20; i++) {
          _client->loop();
          delay(5);
        }
        publishOhautDetails();
        publishOhautNode("online", "1");
    }
    else
    {
      // If we can't connect implement expontial back-off
      reconnect_retry_time = reconnect_retry_time * 2;
      if (reconnect_retry_time > max_reconnect_retry_time)
        reconnect_retry_time = max_reconnect_retry_time;
      Serial.println("MQTTDevice: failed :(");
    }
  }
}
