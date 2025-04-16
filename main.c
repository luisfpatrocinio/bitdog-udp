#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

// Network
#include "lwip/pbuf.h"
#include "lwip/udp.h"

// Patro libs
#include "display.h"
#include "draw.h"
#include "text.h"
#include "buttons.h"

// Math
#include "math.h"

// Time
#include "time.h"

// Definitions
#define UDP_PORT 5000
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "192.168.137.1"
#define BEACON_INTERVAL_MS 1000

#define WIFI_SSID "pat - senha: freertos"
#define WIFI_PASSWORD "freertos"

// Global variables
int pressedTimer = 0;
struct udp_pcb *g_pcb = NULL;

struct repeating_timer timer;

void showError(const char *msg)
{
    clearDisplay();
    drawTextCentered(msg, 8);
    showDisplay();
}

void sendInfo()
{
    static int counter = 0;

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);
    if (!p)
    {
        printf("Failed to allocate pbuf\n");
        return;
    }

    char *req = (char *)p->payload;
    memset(req, 0, BEACON_MSG_LEN_MAX + 1);
    snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);

    err_t er = udp_sendto(g_pcb, p, &addr, UDP_PORT);
    pbuf_free(p);

    if (er != ERR_OK)
    {
        printf("Failed to send UDP packet! error=%d\n", er);
    }
    else
    {
        printf("Sent packet %d\n", counter);
        counter++;
    }
}

void buttonCallback(uint gpio, uint32_t events)
{
    if (pressedTimer > 0)
        return; // Ignore button presses while timer is active

    if (gpio == BTA && isButtonPressed(BTA))
    {
        pressedTimer = 10;
        printf("Button A pressed\n");
        sendInfo();
    }
    else if (gpio == BTB && isButtonPressed(BTB))
    {
        pressedTimer = 10;
        printf("Button B pressed\n");
        sendInfo();
    }
}

bool timerCallback(struct repeating_timer *t)
{
    static int counter = 0;
    if (counter % 10 == 0)
    {
        sendInfo();
    }
    counter++;
    return true; // Keep the timer running
}

void setup()
{
    stdio_init_all();
    initI2C();
    initDisplay();
    initButtons();
    setButtonCallback(buttonCallback);
}

int main()
{
    setup();

    if (cyw43_arch_init())
    {
        printf("Failed to init CYW43\n");
        showError("Failed to init CYW43");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");
    showError("Connecting to Wi-Fi...");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000))
    {
        printf("Failed to connect to Wi-Fi\n");
        showError("Failed to connect to Wi-Fi");
        return 1;
    }
    else
    {
        printf("Connected.\n");
        showError("Connected.");
        sleep_ms(269);
    }

    g_pcb = udp_new();
    if (!g_pcb)
    {
        printf("Failed to create UDP PCB\n");
        showError("PCB fail");
        return 1;
    }

    add_repeating_timer_ms(69, timerCallback, NULL, &timer);

    while (true)
    {
        pressedTimer = MAX(pressedTimer - 1, 0);
        clearDisplay();
        drawTextCentered("Hello Patro", 2);

        // instructions
        drawText(0, 20, "Press A to send UDP packet");
        char buf[32];
        snprintf(buf, sizeof(buf), "Pressed: %d", pressedTimer);
        drawText(0, 28, buf);

        int _spd = 10 + cos(time_us_32() / 1000000.0 * 2 * M_PI) * 3;
        int _amp = 4 + sin(time_us_32() / 1000000.0 * 2 * M_PI) * 2;
        drawWave(SCREEN_HEIGHT - 8, _spd, _amp);

        showDisplay();
    }

    cyw43_arch_deinit();
}
