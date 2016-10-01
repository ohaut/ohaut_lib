#include <Arduino.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
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

void HTTPUpdateServer::setup(ESP8266WebServer *server)
{
    _server = server;

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
