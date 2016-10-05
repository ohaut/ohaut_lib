#include <WiFiClient.h>
#include <ESP8266SSDP.h>
#include <ESP8266WebServer.h>

void SSDP_setup(ESP8266WebServer *server, const char* name, const char *product) {

    server->on("/description.xml", HTTP_GET, [server](){
      SSDP.schema(server->client());
    });

    char tmp_serial[15];
    sprintf(tmp_serial, "esp8266-%06x", ESP.getChipId());

    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName(name);
    SSDP.setSerialNumber(tmp_serial);
    SSDP.setURL("/");
    SSDP.setModelName(product);
    SSDP.setModelNumber("0000001");
    SSDP.setModelURL("http://ohaut.org/");
    SSDP.setManufacturer("Open Home AUTomation");
    SSDP.setManufacturerURL("http://ohaut.org");
    SSDP.begin();
}

