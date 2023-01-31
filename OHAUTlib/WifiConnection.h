#ifndef __WIFI_CONNECTION_H
#define __WIFI_CONNECTION_H
#include <WiFiClient.h>

#include "ConfigMap.h"

extern ConfigMap configData;
typedef enum {
    START = 0,
    TRY_STA = 1,
    STA_CONNECTED = 2,
    AP_MODE = 3,
} tWifiConnectionState;

#define WIFI_VOID_CALLBACK(callback)         void (*callback)()

class WifiConnection {
  public:
    // Events that can be consumed by devices
    WIFI_VOID_CALLBACK(on_wifi_connected);
    /*      This handler is called once the device has wifi connectivity
     *      as an STA device (not when in Access Point mode)
     */
    WIFI_VOID_CALLBACK(on_wifi_disconnected);
    /*      This handler is called once the device has wifi connectivity
     *      as an STA device (not when in Access Point mode)
     */

  
    bool connected();

    void setup();
    void handle();

    WifiConnection();
    ~WifiConnection();

  private:

    void handleStart();
    void handleSTAConnected();
    void handleTrySTA();
    void handleAPMode();

    unsigned long sinceLastTransition();
    void transitionTo(tWifiConnectionState state);
    tWifiConnectionState _state;
    unsigned long _last_transition;
    unsigned long _last_timestamp;
    
};
#endif
