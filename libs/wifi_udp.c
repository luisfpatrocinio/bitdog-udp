/**
 * @file wifi_udp.c
 * @brief Implementation file for Wi-Fi and UDP communication functionalities on RP2040.
 *
 * This file implements the functions declared in wifi_udp.h for initializing
 * and managing a Wi-Fi connection, and for sending UDP packets using the LwIP stack
 * and Pico SDK for CYW43 wireless chip interaction.
 *
 * @author Luis Felipe Patrocinio (https://github.com/luisfpatrocinio)
 */
#include "wifi_udp.h"
#include <string.h> // Required for strlen and memcpy
#include <stdio.h>  // Required for printf

// --- Global Variable Definitions ---

/**
 * @brief Global UDP Protocol Control Block (PCB).
 * @details This PCB is used for UDP communication. It needs to be initialized
 * using `udp_new()` and configured before use.
 */
struct udp_pcb *gPCB = NULL;

/**
 * @brief Global repeating timer for sending UDP data periodically.
 * @details This timer is declared but its setup and callback implementation
 * are not included in this snippet.
 */
struct repeating_timer sendUDPTimer;

/**
 * @brief Global IP address for the UDP target.
 * @details Initialized to 0. Needs to be set to a valid IP address
 * (e.g., using `ipaddr_aton()`) before sending UDP packets.
 */
ip_addr_t gTargetIP = {0}; // Or IP4_ADDR_ANY_INIT for an "any" address if appropriate at init

// --- Function Implementations ---

/**
 * @brief Initializes the CYW43 wireless chip.
 * @note This function must be called before any other Wi-Fi or network operations.
 * It calls `cyw43_arch_init()` to initialize the Wi-Fi hardware and LwIP stack.
 * @return 0 on success, 1 on failure.
 */
int wifiInitCYW43()
{
    // Initialize the CYW43 chip
    if (cyw43_arch_init())
    {
        printf("Failed to initialize CYW43.\n");
        return 1; // Error
    }
    return 0; // Success
}

/**
 * @brief Enables Wi-Fi Station (STA) mode.
 * @note Configures the Wi-Fi module to operate as a station (client), allowing it
 * to connect to an Access Point. This should be called after `wifiInitCYW43()`.
 */
void wifiEnableSTAMode()
{
    // Enable station mode
    cyw43_arch_enable_sta_mode();
}

/**
 * @brief Performs overall basic Wi-Fi setup.
 * @details This function orchestrates calls to `wifiInitCYW43()` and `wifiEnableSTAMode()`.
 */
void wifiSetup()
{
    wifiInitCYW43();
    wifiEnableSTAMode();
    // Note: Further LwIP initializations or CYW43 configurations might be needed
    // depending on the application's complexity (e.g., setting hostname).
}

/**
 * @brief Attempts to connect to a Wi-Fi network with a specified timeout.
 * @param ssid The SSID (network name) of the Wi-Fi network.
 * @param password The password for the Wi-Fi network. Can be NULL for open networks.
 * @param timeout_ms Timeout duration in milliseconds for the connection attempt.
 * @return true if connection is successful within the timeout.
 * @return false if connection fails or times out.
 * @note The authentication type is hardcoded to `CYW43_AUTH_WPA2_AES_PSK`.
 */
bool wifiConnectTimeout(const char *ssid, const char *password, int timeout_ms)
{
    printf("Connecting to SSID (Timeout): %s\n", ssid);
    // Consider making CYW43_AUTH_WPA2_AES_PSK a parameter or configurable
    // if other authentication types are needed.
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

/**
 * @brief Attempts to connect to a Wi-Fi network asynchronously.
 * @param ssid The SSID (network name) of the Wi-Fi network.
 * @param password The password for the Wi-Fi network. Can be NULL for open networks.
 * @return true if the connection attempt was initiated successfully.
 * @return false if there was an error initiating the connection.
 * @note The return value indicates if the connection process started, not if it succeeded.
 * Use `wifiIsConnected()` or `wifiGetStatus()` to check actual connection status.
 * The authentication type is hardcoded to `CYW43_AUTH_WPA2_AES_PSK`.
 */
bool wifiConnectAsync(const char *ssid, const char *password)
{
    printf("Connecting to SSID (Async): %s\n", ssid);
    // Consider making CYW43_AUTH_WPA2_AES_PSK a parameter or configurable.
    if (cyw43_arch_wifi_connect_async(ssid, password, CYW43_AUTH_WPA2_AES_PSK))
    {
        // The API returns 0 on success for async connection initiation.
        // The condition here implies non-zero is failure, which seems correct for the SDK.
        printf("Failed to initiate Wi-Fi connection\n"); // Clarified message
        return false;
    }
    else
    {
        printf("Trying to connect to Wi-Fi...\n");
        return true;
    }
}

/**
 * @brief Checks if the Wi-Fi is currently connected to an Access Point.
 * @return true if the Wi-Fi link is up (connected).
 * @return false if the Wi-Fi link is down (not connected).
 * @note Uses `cyw43_tcpip_link_status()` for the check.
 */
bool wifiIsConnected()
{
    return cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) == CYW43_LINK_UP;
}

/**
 * @brief Disconnects from the current Wi-Fi network.
 */
void wifiDisconnect()
{
Example:
    cyw43_wifi_leave(&cyw43_state, CYW43_ITF_STA);
    printf("Wi-Fi disconnected.\n");
}

/**
 * @brief Gets the current Wi-Fi connection status code.
 * @return int The Wi-Fi link status code (e.g., CYW43_LINK_UP, CYW43_LINK_DOWN,
 * CYW43_LINK_JOIN, CYW43_LINK_FAIL, CYW43_LINK_NONET, CYW43_LINK_BADAUTH).
 * @note Uses `cyw43_wifi_link_status()`.
 */
int wifiGetStatus()
{
    return cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
}

/**
 * @brief Sends a UDP message to a pre-configured target IP and port.
 * @param msg The null-terminated string message to send.
 * @return true if the message was successfully queued for sending by LwIP.
 * @return false if an error occurred (e.g., PCB not initialized, pbuf allocation failed, send error).
 * @note `gTargetIP` and `UDP_PORT` (from header) are used as destination.
 * Ensure `gPCB` is initialized and `gTargetIP` is set before calling.
 */
bool sendUDP(const char *msg)
{
    if (!gPCB)
    {
        printf("[UDP] Error: PCB not initialized.\n");
        return false;
    }
    if (ip_addr_isany_val(gTargetIP) || ip4_addr_isbroadcast_u32(ip4_addr_get_u32(ip_2_ip4(&gTargetIP)), netif_default))
    { // Basic check, might need refinement
        printf("[UDP] Error: Target IP is not a valid unicast address or not set.\n");
        // This check is simplified. For broadcast, `ip_addr_isbroadcast` would be true.
        // If broadcast is an intended use case for this specific function, the check needs adjustment.
        // For now, assuming unicast target for `sendUDP`.
    }

    // `addr` is a local copy, this is fine.
    ip_addr_t addr = gTargetIP;

    printf("[UDP] Sending to %s:%d\n", ipaddr_ntoa(&addr), UDP_PORT);

    // Allocate pbuf with space for null terminator, though UDP doesn't strictly need it if length is managed.
    // PBUF_TRANSPORT is correct for UDP payload.
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg) + 1, PBUF_RAM);
    if (!p)
    {
        printf("[UDP] Error: Failed to allocate pbuf\n");
        return false;
    }

    // Copy message to pbuf payload.
    // Using strlen(msg) + 1 to include null terminator. If receiver doesn't expect it,
    // or if using fixed-length messages, just strlen(msg) is enough for pbuf_alloc and memcpy.
    memcpy(p->payload, msg, strlen(msg) + 1);

    // Send the UDP packet.
    err_t er = udp_sendto(gPCB, p, &addr, UDP_PORT);

    // Free the pbuf. This must be done regardless of send success or failure.
    pbuf_free(p);

    if (er != ERR_OK)
    {
        printf("[UDP] Error sending UDP packet: %d\n", er);
        return false;
    }

    return true;
}

/**
 * @brief Creates a new UDP PCB and binds it to the `UDP_BROADCAST_PORT` for receiving messages.
 * @note This function initializes `gPCB` if it's NULL by calling `udp_new()`.
 * It then binds the PCB to `IP_ANY_TYPE` and `UDP_BROADCAST_PORT`.
 * A UDP receive callback should be set up using `udp_recv()` after this function
 * if incoming packets on this port need to be processed.
 * This function currently uses `UDP_BROADCAST_PORT` for binding, which might
 * differ from `UDP_PORT` used for sending. Clarify intent if they should be the same.
 */
void openUDPBind()
{
    if (gPCB == NULL)
    {
        gPCB = udp_new();
        if (gPCB == NULL)
        {
            printf("[UDP] Failed to create new PCB\n");
            return;
        }
    }
    // else: PCB already exists, assume it can be re-bound or this is an error.
    // For simplicity, current code will reuse/rebind if gPCB is not NULL.

    printf("[UDP] Binding to port %d\n", UDP_BROADCAST_PORT);

    // Binds to any local IP address.
    err_t err = udp_bind(gPCB, IP_ANY_TYPE, UDP_BROADCAST_PORT);
    if (err != ERR_OK)
    {
        printf("[UDP] Failed to bind PCB to port %d. Error: %d\n", UDP_BROADCAST_PORT, err);
        // Consider cleaning up gPCB if binding fails and it was newly created.
        // udp_remove(gPCB); gPCB = NULL;
    }
    else
    {
        printf("[UDP] Bound successfully to port %d\n", UDP_BROADCAST_PORT);
    }
    // Note: To receive UDP packets, a receive callback must be registered:
    // udp_recv(gPCB, your_udp_receive_callback, NULL);
}