#include <ESP8266WiFi.h>
#include "consts.h"
extern ConfigMap configData;
bool wifiSetup()
{
  int connect_tries=3;

  WiFi.mode(WIFI_STA);
  WiFi.hostname(configData["mqtt_id"]);
  if (strcmp(configData["wifi_sta_ap"], DEFAULT_STA_AP) == 0 &&
      strcmp(configData["wifi_sta_pass"], DEFAULT_STA_PASS) == 0) {
    // if no specific ap/password is set, try first with the SDK
    // stored settings
    WiFi.begin();
  } else {
    WiFi.begin(configData["wifi_sta_ap"],
               configData["wifi_sta_pass"]);
  }

  while(--connect_tries && WiFi.waitForConnectResult() != WL_CONNECTED){
    WiFi.begin(configData["wifi_sta_ap"],
               configData["wifi_sta_pass"]);
    Serial.println("WiFi failed, retrying.");
  }

  if (connect_tries <= 0) {
    Serial.println("WiFi: setting AP mode");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(configData["wifi_ap_ssid"], configData["wifi_ap_pass"]);
    return false;
  }
  return true;
}
