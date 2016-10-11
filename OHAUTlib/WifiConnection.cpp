#include <ESP8266WiFi.h>
#include "ConfigMap.h"
#include "consts.h"
extern ConfigMap configData;
bool wifi_connected = false;

// TODO(mangelajo): convert this to a proper class...

bool wifiSetup()
{
  int connect_tries=3;

  WiFi.softAP(configData["wifi_ap_ssid"], configData["wifi_ap_pass"]);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(configData["mqtt_id"]);

  WiFi.begin(configData["wifi_sta_ap"],
             configData["wifi_sta_pass"]);

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
  } else {
    Serial.printf("WiFi: connected to %s, IP address: %s\r\n",
            configData["wifi_sta_ap"],
            WiFi.localIP().toString().c_str());

  }

  wifi_connected = true;
  return true;
}
