#ifndef __HTTP_UPDATE_SERVER_H
#define __HTTP_UPDATE_SERVER_H
#include <FS.h>

class ESP8266WebServer;

class HTTPUpdateServer
{
  private:
    File _appFile;
    ESP8266WebServer *_server;
    static const char *_serverIndex;
  public:
    HTTPUpdateServer();
    void setup(ESP8266WebServer *server=NULL);
};


#endif
