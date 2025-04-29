#ifndef WIFI_UDP_H
#define WIFI_UDP_H

// Network
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "pico/cyw43_arch.h"

#define UDP_PORT 5000
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "192.168.137.1"
#define BEACON_INTERVAL_MS 1000

#define WIFI_SSID "Patrocinio"
#define WIFI_PASSWORD "patropatro"

#define UDP_BROADCAST_PORT 1234

extern struct udp_pcb *gPCB;
extern struct repeating_timer sendUDPTimer;
extern ip_addr_t gTargetIP;

void wifiInitCYW43();
void wifiEnableSTAMode();
void wifiSetup();

bool wifiConnectTimeout(const char *ssid, const char *password, int timeout_ms);
bool wifiConnectAsync(const char *ssid, const char *password);

bool wifiIsConnected();

void wifiDisconnect();

int wifiGetStatus();

bool sendUDP(const char *msg);

void openUDPBind();

#endif // WIFI_UDP_H