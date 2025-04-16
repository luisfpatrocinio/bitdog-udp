#include "wifi_udp.h"

struct udp_pcb *gPCB = NULL;
struct repeating_timer sendUDPTimer;

void wifiInitCYW43()
{
    // Initialize the CYW43 chip
    if (cyw43_arch_init())
    {
        printf("Failed to initialize CYW43\n");
        return;
    }
}

void wifiEnableSTAMode()
{
    // Enable station mode
    cyw43_arch_enable_sta_mode();
}

void wifiSetup()
{
    wifiInitCYW43();
    wifiEnableSTAMode();
    wifiConnect(WIFI_SSID, WIFI_PASSWORD);
}

bool wifiConnect(const char *ssid, const char *password)
{
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to Wi-Fi\n");
        return false;
    }
    else
    {
        printf("Connected.\n");
        return true;
    }
}

void wifiDisconnect()
{
    // not implemented
}