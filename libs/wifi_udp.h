#ifndef WIFI_UDP_H
#define WIFI_UDP_H

// Network
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "pico/cyw43_arch.h"

#define UDP_PORT 5000
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "192.168.137.1"
#define BEACON_INTERVAL_MS 1000

#define WIFI_SSID "pat - senha: freertos"
#define WIFI_PASSWORD "freertos"

extern struct udp_pcb *gPCB;
extern struct repeating_timer sendUDPTimer;

void wifiInitCYW43();
void wifiEnableSTAMode();

void wifiSetup();
bool wifiConnect(const char *ssid, const char *password);
void wifiDisconnect();

#endif // WIFI_UDP_H