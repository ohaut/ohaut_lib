#ifndef __HTTP_UPDATE_SERVER_H
#define __HTTP_UPDATE_SERVER_H
#include <FS.h>

class OHAUTservice;

class HTTPUpdateServer
{
  private:
    File _appFile;
    ESP8266WebServer *_server;
    OHAUTservice *_ohaut;
    static const char *_serverIndex;

    int _update_status;
    String _getSPIFFSversion();
    void _handleGetUpdateStatus();
    bool _downloadAppHtmlGz(const char* url);
    void _handleUpdateAppHtmlGz();
    void _handleUpdateSPIFFS();
    void _handleUpdateFirmware();
    void _handleUpdateAll();

  public:
    HTTPUpdateServer();
    void setup(ESP8266WebServer *server, OHAUTservice *ohaut);
};


#endif
