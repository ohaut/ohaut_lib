#include <FS.h>
#include "ConfigMap.h"
#include <Ticker.h>
#include <functional>
#include "consts.h"
#include <OHAUTlib.h>

ConfigMap configData;
WebServer* _server;
//FIXME: remove globals, etc.
extern bool wifi_connected;

bool handleFileRead(WebServer *server, String path);

void setDefaultConfig() {
  char esp_id[32];

  // create an unique ID for the AP SSID and MQTT ID
#ifdef ESP8266
  sprintf(esp_id, "OHAUT_%08x", ESP.getChipId());
#elif defined(ESP32)
  sprintf(esp_id, "OHAUT_%08x", (uint32_t)ESP.getEfuseMac());
#endif

  // WiFi
  configData.set("wifi_sta_ap", DEFAULT_STA_AP);
  configData.set("wifi_sta_pass", DEFAULT_STA_PASS);

  configData.set("wifi_ap_ssid", esp_id);
  configData.set("wifi_ap_pass", "dimmer123456");


  configData.set("host_id", esp_id);

  // OHAUT integration
  configData.set("oh_int", "1");
  configData.set("oh_room", "Default");
  configData.set("oh_section", "");
  configData.set("oh_order", "");
  configData.set("oh_name", esp_id);
}

void upgradeConfig() {
  char esp_id[32];
  // create an unique ID for the AP SSID and MQTT ID
#ifdef ESP8266
  sprintf(esp_id, "OHAUT_%08x", ESP.getChipId());
#elif defined(ESP32)
  sprintf(esp_id, "OHAUT_%08x", (uint32_t)ESP.getEfuseMac());
#endif
  char *cfg = configData["mqtt_id"];
  if (cfg != NULL && (configData["host_id"] == NULL || strcmp(configData["host_id"], esp_id) == 0 )) {
    configData.set("host_id", cfg);
  }
}

void configSetup(CONFIG_CALLBACK(callback)) {
  #ifdef ESP8266
  SPIFFS.begin();
  #elif defined(ESP32)
  SPIFFS.begin(true /* Auto Format enabled */);
  #endif

  SPIFFS.begin();
  setDefaultConfig();
  if (callback) callback(&configData);
  configData.readTSV(CONFIG_FILENAME);
  upgradeConfig();
}

void handleConfig() {
  String form =
    R"(<html>
      <head>
        <style>
          label { float:left; padding-left:2em; width: 12em; }
         </style>
      <body>
       <h1>Configuration</h1>
       <form action="/config" method="POST">
        <h2>WiFi settings</h2>
        <div><label>WiFi ssid:</label><input name='wifi_sta_ap' value='$wifi_sta_ap'></div>
        <div><label>WiFi password:</label><input name='wifi_sta_pass' type='password' value='$wifi_sta_pass'></div>
        <h3>WiFi AP (fallback config)</h3>
        <div><label>AP ssid:</label><input name='wifi_ap_ssid' value='$wifi_ap_ssid'></div>
        <div><label>AP password:</label><input name='wifi_ap_pass' type='password' value='$wifi_ap_pass'></div>

          <input type="submit" value="Save and Reboot">
       </form>
         My WiFi STA IP: $IP
         $download_app
       </body>
      </html>)";


  String download_app = "";

  download_app = "<a href=\"/update/app\">download html interface app</a>";

  form.replace("$download_app", download_app);
  configData.replaceVars(form);
  form.replace("$IP", WiFi.localIP().toString());
  _server->send(200, "text/html", form);
}

// convert a single hex digit character to its integer value
unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}
void urldecode(char *urlbuf)
{
    char c;
    char *dst;
    dst=urlbuf;
    while ((c = *urlbuf)) {
        if (c == '+') c = ' ';
        if (c == '%') {
            urlbuf++;
            c = *urlbuf;
            urlbuf++;
            c = (h2int(c) << 4) | h2int(*urlbuf);
        }
        *dst = c;
        dst++;
        urlbuf++;
    }
    *dst = '\0';
}


int endsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}


void handlePost(std::function<void()> render_form_function) {
  /* Grab every known config entry from the POST request arguments,
   * urldecode it, and set it back to the config object.
   */
  configData.foreach(
      [](const char* key, const char* value, bool last) {
        if (_server->hasArg(key)) {
          String val = _server->arg(key);
          const char *_str = val.c_str();
           Serial.printf("set %s = %s\r\n", key, _str);
          if (_str) {
            char *str = strdup(_str);
            urldecode(str);
            configData.set(key, str);
            free(str);
          }
       } else {
            if (endsWith(key, "_bool")) {
              Serial.printf("key %s is bool\r\n", key);
              char *str = strdup("false");
              configData.set(key, str);
              free(str);
            }
          }
      }
  );

  /* write a TSV file */
  configData.writeTSV(CONFIG_FILENAME);

  /* return the form again with the new data */
  render_form_function();

  /* allow for the data to be sent back to browser */
  for (int i=0;i<100;i++){
    yield();
    delay(10);
  }
  ESP.restart();
}

void redirectToConfig() {
  _server->sendHeader("Location", "/config");
  _server->send(307);
}

void handleConfigPost() {
  handlePost(handleConfig);
}


void handleConfigGet() {
   _server->send(200, "text/html", configData.toJsonStr());
}

void handleConfigSave() {
  handlePost([]{
    _server->send(200, "application/json", "{\"result\": \"0\","
                                           " \"message\": \"saved ok\"}");
  });
}

void handleReboot() {
  _server->send(200, "text/html", "{\"result\": \"0\","
                                   "\"message\": \"rebooting\"}");
  /* allow for the data to be sent back to browser */
  delay(1000);
  ESP.restart();
}


void configServerSetup(WebServer *server,
                       CONFIG_CALLBACK(cfg_callback)) {
  _server = server;
  configSetup(cfg_callback);

  server->on("/", HTTP_GET, [server](){
    if(!handleFileRead(server, "/app.html")) {
      redirectToConfig();
    };
  });
  server->on("/config.json", HTTP_GET, handleConfigGet);
  server->on("/config.json", HTTP_POST, handleConfigSave);
  server->on("/config", HTTP_GET, handleConfig); // fallback handler for no APP
  server->on("/config", HTTP_POST, handleConfigPost);
  server->on("/reboot", HTTP_GET, handleReboot);

  // static file serving
  server->onNotFound([server](){
     if(!handleFileRead(server, server->uri()))
    server->send(404, "text/plain", "File not found c(^_^)ç");
   });
}
