/**
 * @file wifi_udp.h
 * @brief Header file for Wi-Fi and UDP communication functionalities on RP2040.
 *
 * This file defines constants, declares global variables, and prototypes functions
 * for initializing and managing a Wi-Fi connection, and for sending UDP packets.
 * It utilizes the LwIP stack and the Pico SDK for CYW43 wireless chip interaction.
 *
 * @author Luis Felipe Patrocinio (https://github.com/luisfpatrocinio)
 */
#ifndef WIFI_UDP_H
#define WIFI_UDP_H

// Network
#include "lwip/pbuf.h"       ///< LwIP: Packet buffer management.
#include "lwip/udp.h"        ///< LwIP: UDP protocol functions and structures.
#include "lwip/netif.h"      ///< LwIP: Network interface management.
#include "pico/cyw43_arch.h" ///< Pico SDK: Architecture for CYW43 Wi-Fi chip integration with LwIP.

// --- Configuration Constants ---

#define UDP_PORT 5000                 ///< Default UDP port for communication.
#define BEACON_MSG_LEN_MAX 127        ///< Maximum length for a beacon message.
#define BEACON_TARGET "192.168.137.1" ///< Default target IP address for beacon messages.
#define BEACON_INTERVAL_MS 1000       ///< Interval in milliseconds for sending beacon messages.

#define WIFI_SSID "patro"               ///< Default Wi-Fi SSID.
#define WIFI_PASSWORD "cafecombiscoito" ///< Default Wi-Fi password.

#define UDP_BROADCAST_PORT 1234 ///< UDP port for broadcast messages.

// --- Global Variables ---

extern struct udp_pcb *gPCB;                ///< Global UDP Protocol Control Block.
extern struct repeating_timer sendUDPTimer; ///< Global repeating timer for sending UDP data periodically.
extern ip_addr_t gTargetIP;                 ///< Global IP address for the UDP target.

// --- Function Prototypes ---

/**
 * @brief Initializes the CYW43 wireless chip.
 *
 * This function must be called before any other Wi-Fi or network operations.
 * It typically initializes the underlying hardware and drivers for the Wi-Fi module.
 *
 * @return 0 on success, 1 on failure.
 */
int wifiInitCYW43();

/**
 * @brief Enables Wi-Fi Station (STA) mode.
 *
 * Configures the Wi-Fi module to operate as a station, allowing it to connect
 * to an Access Point.
 */
void wifiEnableSTAMode();

/**
 * @brief Performs overall Wi-Fi setup.
 *
 * This function might orchestrate calls to `wifiInitCYW43` and `wifiEnableSTAMode`,
 * and potentially other initial configurations.
 */
void wifiSetup();

/**
 * @brief Attempts to connect to a Wi-Fi network with a timeout.
 *
 * @param ssid The SSID (network name) of the Wi-Fi network.
 * @param password The password for the Wi-Fi network. Can be NULL for open networks.
 * @param timeout_ms Timeout duration in milliseconds for the connection attempt.
 * @return true if connection is successful within the timeout.
 * @return false if connection fails or times out.
 */
bool wifiConnectTimeout(const char *ssid, const char *password, int timeout_ms);

/**
 * @brief Attempts to connect to a Wi-Fi network asynchronously.
 *
 * This function initiates a connection attempt and returns immediately.
 * The connection status should be checked using `wifiIsConnected()` or `wifiGetStatus()`.
 * @param ssid The SSID (network name) of the Wi-Fi network.
 * @param password The password for the Wi-Fi network. Can be NULL for open networks.
 * @return true if the connection attempt was initiated successfully.
 * @return false if there was an error initiating the connection (e.g., invalid parameters).
 * @note The return value does not indicate a successful connection, only that the process started.
 */
bool wifiConnectAsync(const char *ssid, const char *password);

/**
 * @brief Checks if the Wi-Fi is currently connected to an Access Point.
 *
 * @return true if the Wi-Fi is connected.
 * @return false if the Wi-Fi is not connected.
 */
bool wifiIsConnected();

/**
 * @brief Disconnects from the current Wi-Fi network.
 */
void wifiDisconnect();

/**
 * @brief Gets the current Wi-Fi connection status.
 *
 * The returned integer typically corresponds to status codes defined by the
 * CYW43 architecture or LwIP (e.g., CYW43_LINK_UP, CYW43_LINK_DOWN, etc.).
 * @return int The Wi-Fi connection status code.
 */
int wifiGetStatus();

/**
 * @brief Sends a UDP message to the pre-configured target IP and port.
 *
 * The target IP is usually `gTargetIP` and the port is `UDP_PORT`.
 * @param msg The null-terminated string message to send.
 * @return true if the message was successfully queued for sending.
 * @return false if an error occurred (e.g., message too long, PCB not ready).
 */
bool sendUDP(const char *msg);

/**
 * @brief Opens and binds a UDP PCB to the `UDP_PORT`.
 *
 * This function sets up the local endpoint for receiving UDP packets on `UDP_PORT`.
 * It initializes or configures `gPCB`.
 * @note A UDP receive callback should be set up elsewhere using `udp_recv()` if incoming
 * packets on this port need to be processed.
 */
void openUDPBind();

#endif // WIFI_UDP_H