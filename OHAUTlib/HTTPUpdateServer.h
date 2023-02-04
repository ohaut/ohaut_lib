#ifndef __HTTP_UPDATE_SERVER_H
#define __HTTP_UPDATE_SERVER_H
#include <FS.h>

class OHAUTservice;

class HTTPUpdateServer
{
  private:
    File _appFile;
    WebServer *_server;
    OHAUTservice *_ohaut;
    static const char *_serverIndex;

    int _update_status;
    String _getSPIFFSversion();
    void _handleGetUpdateStatus();
    bool _downloadAppHtmlGz(const char* url);
    bool _downloadFirmware();
    void _handleUpdateAppHtmlGz();
    void _handleUpdateFirmware();
    void _handleUpdateAll();

  public:
    HTTPUpdateServer();
    void setup(WebServer *server, OHAUTservice *ohaut);
};


#endif
