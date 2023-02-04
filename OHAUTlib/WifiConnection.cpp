
#ifdef ESP8266
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <Wifi.h>
#endif
#include "ConfigMap.h"
#include "consts.h"

#include "WifiConnection.h"

extern ConfigMap configData;

WifiConnection::WifiConnection() {
  this->on_wifi_connected = NULL;
  this->on_wifi_disconnected = NULL;
}

WifiConnection::~WifiConnection() {

}

void WifiConnection::setup() {
  Serial.printf("Wifi[setup]: My host id is: %s\r\n", configData["host_id"]);
  WiFi.hostname(configData["host_id"]);
  transitionTo(START);
  handle(); // do the start.... (some other parts like OTA seem to
            // need WiFi initialized otherwise it crashes)
}

void WifiConnection::handle() {
  switch(_state) {
    case START:         handleStart();
                        break;
    case TRY_STA:       handleTrySTA();
                        break;
    case STA_CONNECTED: handleSTAConnected();
                        break;
    case AP_MODE:       handleAPMode();
                        break;
    default:
      _state = START;
  }
}

bool WifiConnection::connected() {
  return _state == STA_CONNECTED;
}

void WifiConnection::transitionTo(tWifiConnectionState state) {
  _state = state;
  _last_transition = millis();
  _last_timestamp = _last_transition;

  switch(_state) {
    case AP_MODE:
              WiFi.mode(WIFI_AP);
              WiFi.softAP(configData["wifi_ap_ssid"], configData["wifi_ap_pass"]);
              break;
  }
}

unsigned long WifiConnection::sinceLastTransition() {
  return millis() - _last_transition;
}

void WifiConnection::handleStart() {

  WiFi.mode(WIFI_STA);
  
  Serial.printf("Wifi[start]: Connecting to AP %s\r\n", configData["wifi_sta_ap"]);

  WiFi.begin(configData["wifi_sta_ap"], configData["wifi_sta_pass"]);
  transitionTo(TRY_STA);
}

void WifiConnection::handleTrySTA() {

  if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("WiFi[try_sta] connected to %s, IP address: %s\r\n",
          configData["wifi_sta_ap"],
          WiFi.localIP().toString().c_str());
      transitionTo(STA_CONNECTED);
      if (this->on_wifi_connected) this->on_wifi_connected();

  } else if (sinceLastTransition() > 5000) {
      Serial.println("Wifi[try_sta]: timed out..., switching to AP mode.");
      transitionTo(AP_MODE);
  }
}

void WifiConnection::handleSTAConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    _last_timestamp = millis();
    return;
  } 

  if ((millis()-_last_timestamp)>10000) {
      Serial.println("Wifi[sta_connected]: disconnection timed out..., switching to AP mode.");
      transitionTo(AP_MODE);
      if (this->on_wifi_disconnected) this->on_wifi_disconnected();
  }
}

void WifiConnection::handleAPMode() {
    if (WiFi.softAPgetStationNum()>0) {
      _last_timestamp = millis();
      return;
    }

    /* after one minute with no stations connected, we try again to connect to the WiFi*/
    if ((millis()-_last_timestamp)> 60000 ) {
        Serial.println("WiFi[ap_mode] no stations connected for a while, retrying connection.");
        transitionTo(START);
    }
}
