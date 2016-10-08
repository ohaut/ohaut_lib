#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include "OHAUTlib.h"
#include "consts.h"
#include "HTTPUpdateServer.h"

const char* HTTPUpdateServer::_serverIndex =
R"(<html><body>
    <form method='POST' action='/update-app/' enctype='multipart/form-data'>
          <input type='file' name='update'>
          <input type='submit' value='Update'>
    </form>
    </body></html>)";

HTTPUpdateServer::HTTPUpdateServer()
{
  _server = NULL;
}

void HTTPUpdateServer::setup(ESP8266WebServer *server, OHAUTservice *ohaut)
{
    _server = server;
    _ohaut = ohaut;

    _server->on("/update/status", HTTP_GET,   [&](){ _handleGetUpdateStatus(); });
    _server->on("/update/spiffs", HTTP_GET,   [&](){ _handleUpdateSPIFFS();    });
    _server->on("/update/firmware", HTTP_GET, [&](){ _handleUpdateFirmware();  });
    _server->on("/update/app", HTTP_GET,      [&](){ _handleUpdateAppHtmlGz(); });
    _server->on("/update/all", HTTP_GET,      [&](){ _handleUpdateAll();       });

    // handler for the /update form page
    _server->on("/update-app/", HTTP_GET, [&](){
      _server->sendHeader("Connection", "close");
      _server->sendHeader("Access-Control-Allow-Origin", "*");
      _server->send(200, "text/html", _serverIndex);
    });

    // handler for the /update form POST (once file upload finishes)
    _server->on("/update-app/", HTTP_POST, [&](){
      _server->sendHeader("Connection", "close");
      _server->sendHeader("Access-Control-Allow-Origin", "*");
      _server->send(200, "text/html", "<META http-equiv=\"refresh\" content=\"2;URL=/\">DONE");
      ESP.restart();
    },[&](){
      // handler for the file upload, get's the sketch bytes, and writes
      // them through the Update object
      HTTPUpload& upload = _server->upload();
      if(upload.status == UPLOAD_FILE_START){
          WiFiUDP::stopAll();
          _appFile = SPIFFS.open("/app.html.gz_", "w");
      } else if(upload.status == UPLOAD_FILE_WRITE){
        _appFile.write(upload.buf, upload.currentSize);
      } else if(upload.status == UPLOAD_FILE_END){
        _appFile.close();
        SPIFFS.remove("/app.html.gz");
        SPIFFS.rename("/app.html.gz_", "/app.html.gz");
      } else if(upload.status == UPLOAD_FILE_ABORTED){
        _appFile.close();
        SPIFFS.remove("/app.html.gz_");
      }
      yield();
    });
}

String HTTPUpdateServer::_getSPIFFSversion() {
    bool waiting_first_comma = true;
    bool reading_version = false;
    String result = "";
    File versionFile = SPIFFS.open("/VERSION_H", "r");

    if (!versionFile) goto error;

    while (versionFile.available()) {
      int val = versionFile.read();
      if (((char)val == '"') && reading_version) {
        versionFile.close();
        return result;
      }
      if (((char)val == '"') && waiting_first_comma) {
        reading_version = true;
        waiting_first_comma = false;
        continue;
      }
      if (reading_version) {
        result += (char) val;
      }
    }
    versionFile.close();
error:
    result = "NOTFOUND";
    return result;
}

void HTTPUpdateServer::_handleGetUpdateStatus()
{
  char result[128];
  String spiffs_version = _getSPIFFSversion();

  sprintf(result, "{\"update_status\": \"%d\", "
                  "\"firmware_version\": \"%s\", "
                  "\"spiffs_version\": \"%s\"}",
          _update_status, _ohaut->get_firmware_version(),
          spiffs_version.c_str());

  _server->send(200, "application/json", result);
}

bool HTTPUpdateServer::_downloadAppHtmlGz(const char* url=NULL) {

    String url_str;
    int len;
    if (!url) {
        url_str = "http://ohaut.org/";
        url_str += _ohaut->get_device_name();
        url_str += "/firmware/master/app.html.gz";
        url = url_str.c_str();
    }

    HTTPClient http;
    http.begin(url);

    int httpCode = http.GET();

    Serial.printf("HTTP code: %d\r\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      len = http.getSize();
      uint8_t buf[1024];
      WiFiClient* stream = http.getStreamPtr();
      WiFiUDP::stopAll();
      WiFiClient::stopAllExcept(stream);

      delay(100);

      File appFile = SPIFFS.open("/app.html.gz_", "w");

      if (!appFile) {
        Serial.println("Error opening file for write");
        return false;
      }

      while(http.connected() && len>0) {
        size_t to_read = stream->available();
        //Serial.printf("HTTP available: %d\r\n", to_read);
        if (to_read) {
          if (to_read>sizeof(buf)) to_read = sizeof(buf);
          int bytes = stream->readBytes(buf, to_read);
          appFile.write(buf, bytes);
          len -= bytes;
        }
      }
      appFile.close();
    }
    else
    {
      Serial.println("Error");
      return false;
    }

    Serial.println("Done");

    if (len <= 0) {
      SPIFFS.remove("/app.html.gz");
      SPIFFS.rename("/app.html.gz_", "/app.html.gz");
      return true;
    }
    return false;
}

void HTTPUpdateServer::_handleUpdateAppHtmlGz() {

  _update_status = 1;
  Serial.println("Updating app.html.gz");

  _server->send(200,"application/json", "{\"result\": \"0\", \"message:\": "
                                      "\"app.html.gz updating, please wait\"}");
  _server->stop();

  if (_downloadAppHtmlGz())
    _update_status = 2;
  else
    _update_status = -1;

  _server->begin();

}

void HTTPUpdateServer::_handleUpdateSPIFFS() {
  String url;
  Serial.println("Updating SPIFFS from HTTP");
  _server->send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                       "\"SPIFFS updating, please wait\"}");
  _server->stop();
  _update_status = 1;
  url = "http://ohaut.org/";
  url += _ohaut->get_device_name() + 
  url += "/firmware/master/spiffs.bin";
  t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(url.c_str());
  if (ret == HTTP_UPDATE_OK) _update_status = 2;
  else _update_status = -1;
  Serial.printf("Updating SPIFFS done: %d", _update_status);
  configData.writeTSV(CONFIG_FILENAME);
  _server->begin();
}

void HTTPUpdateServer::_handleUpdateFirmware() {
  String url;
  Serial.println("Updating Firwmare from HTTP");
  _server->send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                         "\"Firmware updating, please wait\"}");

  _server->stop();
  _update_status = 1;
  url = "http://ohaut.org/";
  url += _ohaut->get_device_name();
  url += "/firmware/master/firmware.bin";
  t_httpUpdate_return ret = ESPhttpUpdate.update(url.c_str());
  if (ret == HTTP_UPDATE_OK) _update_status = 3;
  else _update_status = -1;
  Serial.printf("Updating Firmware done: %d", _update_status);
  _server->begin();
}

void HTTPUpdateServer::_handleUpdateAll() {
  String url;
  Serial.println("Updating ALL:");
  _server->send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                          "\"Updating everything, please wait\"}");

  _server->stop();
  Serial.println(" * App from HTTP");

  _update_status = 1;
  if (_downloadAppHtmlGz()) _update_status = 2;
  else
  {
    _update_status = -1;
    _server->begin();
    return;
  }

  Serial.println(" * Firmware from HTTP");
  url = "http://ohaut.org/";
  url += _ohaut->get_device_name();
  url += "/firmware/master/firmware.bin";

  t_httpUpdate_return ret = ESPhttpUpdate.update(url.c_str());

  if (ret == HTTP_UPDATE_OK) _update_status = 3;
  else _update_status = -1;
  Serial.printf("Updating Firmware done: %d", _update_status);
  _server->begin();
}

//TODO(mangelajo): Add API to scan networks
// https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/WiFiScan/WiFiScan.ino

