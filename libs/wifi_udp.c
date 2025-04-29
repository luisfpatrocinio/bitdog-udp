#include "wifi_udp.h"

struct udp_pcb *gPCB = NULL;
struct repeating_timer sendUDPTimer;
ip_addr_t gTargetIP = {0};

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
}

bool wifiConnectTimeout(const char *ssid, const char *password, int timeout_ms)
{
    printf("Connecting to SSID (Timeout): %s\n", ssid);
    if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, timeout_ms))
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

bool wifiConnectAsync(const char *ssid, const char *password)
{
    printf("Connecting to SSID (Async): %s\n", ssid);
    if (cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK))
    {
        printf("Failed to connect to Wi-Fi\n");
        return false;
    }
    else
    {
        printf("Trying to connect to Wi-Fi...\n");
        return true;
    }
}

bool wifiIsConnected()
{
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

void wifiDisconnect()
{
    // not implemented
}

int wifiGetStatus()
{
    return cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
}

bool sendUDP(const char *msg)
{
    // printf("[UDP] Sending: %s\n", msg);

    ip_addr_t addr = gTargetIP;

    printf("[UDP] Sending to %s:%d\n", ipaddr_ntoa(&addr), UDP_PORT);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg) + 1, PBUF_RAM);
    if (!p)
        return false;

    memcpy(p->payload, msg, strlen(msg) + 1);
    err_t er = udp_sendto(gPCB, p, &addr, UDP_PORT);
    pbuf_free(p);

    return er == ERR_OK;
}

void openUDPBind()
{
    printf("[UDP] Binding to port %d\n", UDP_BROADCAST_PORT);
    udp_bind(gPCB, IP_ANY_TYPE, UDP_BROADCAST_PORT);
}