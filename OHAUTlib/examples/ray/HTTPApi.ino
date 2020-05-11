#include <OHAUTlib.h>
#include "ray_global_defs.h"

WebServer *web_server;

void handleSetLed() {

    int ch , val;
    char* str_ch = strdup(web_server->arg("ch").c_str());
    char* str_val = strdup(web_server->arg("val").c_str());

    if (!strlen(str_val)) { /* we asume no ch means all channels */
      web_server->send(422, "application/json", "{\"result\": \"1\", \"message:\": "
                                            "\"val parameter missing on URL\"}");
      goto _exit;
    }
    ch = atoi(str_ch);
    val = atoi(str_val);

    if (ch<0 || ch>=3) {
      web_server->send(422, "application/json", "{\"result\": \"2\", \"message:\": "
                                            "\"ch out of range (0..2)\"}");
      goto _exit;
    }

    if (val<0 || val>100) {
      web_server->send(422,  "application/json", "{\"result\": \"3\", \"message:\": "
                                            "\"val out of range (0..100)\"}");
      goto _exit;
    }

    if (strlen(str_ch))
       dimmers.setDimmer(ch, ((float)val)/100.0);
    else {
      for (ch=0; ch<N_DIMMERS; ch++)
        dimmers.setDimmer(ch, ((float)val)/100.0);
    }
    web_server->send(200, "application/json", "{\"result\": \"0\", \"message:\": "
                                          "\"channel set correctly\"}");
 _exit:
    free (str_ch);
    free (str_val);
}

void handleGetLeds() {
  char result[128];
  sprintf(result, "{\"ch0\": \"%d\","
                   "\"ch1\": \"%d\","
                   "\"ch2\": \"%d\"}",
                   (int)(dimmers.getDimmer(0)*100.0),
                   (int)(dimmers.getDimmer(1)*100.0),
                   (int)(dimmers.getDimmer(2)*100.0));
  web_server->send(200, "application/json", result);
}

void setupHTTPApi(WebServer *server) {
  web_server = server;
  server->on("/setLed", HTTP_GET,  handleSetLed);
  server->on("/getLeds", HTTP_GET, handleGetLeds);
}
