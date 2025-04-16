#include <stdio.h>
#include "pico/stdlib.h"

// Patro libs
#include "display.h"
#include "draw.h"
#include "text.h"
#include "buttons.h"
#include "led.h"
#include "wifi_udp.h"

// Math
#include "math.h"

// Time
#include "time.h"

// Global variables
int pressedTimer = 0;

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

    err_t er = udp_sendto(gPCB, p, &addr, UDP_PORT);
    pbuf_free(p);

    if (er != ERR_OK)
    {
        printf("Failed to send UDP packet! error=%d\n", er);
    }
    else
    {
        printf("Sent packet %d\n", counter);
        counter++;
        counter = counter % 100;
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
    initLeds();
    setLedBrightness(LED_RED_PIN, 0);

    clearDisplay();
    drawTextCentered("Patro UDP", 16);
    drawTextCentered("Starting...", 24);
    showDisplay();
}

void udpReceiveCallback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (!p)
    {
        printf("Received empty packet\n");
        return;
    }

    // Copia os dados recebidos para uma string (garantindo null terminator)
    char msg[BEACON_MSG_LEN_MAX + 1];
    memset(msg, 0, sizeof(msg));
    memcpy(msg, p->payload, MIN(p->len, BEACON_MSG_LEN_MAX));

    printf("Received UDP packet from %s:%d:\n", ipaddr_ntoa(addr), port);
    printf("Message: %s\n", msg);

    // Mostra no display (opcional)
    clearDisplay();
    drawTextCentered(msg, 4);
    showDisplay();
    setLedBrightness(LED_RED_PIN, 255);
    sleep_ms(300); // Só para dar tempo de ler
    setLedBrightness(LED_RED_PIN, 0);

    pbuf_free(p);
}

int main()
{
    setup();
    wifiSetup();

    gPCB = udp_new();
    if (!gPCB)
    {
        printf("Failed to create UDP PCB\n");
        drawError("PCB fail");
        return 1;
    }

    udp_recv(gPCB, udpReceiveCallback, NULL);

    add_repeating_timer_ms(69, timerCallback, NULL, &sendUDPTimer);

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

        // Wave
        int _spd = 10 + cos(time_us_32() / 1000000.0 * 2 * M_PI) * 3;
        int _amp = 4 + sin(time_us_32() / 1000000.0 * 2 * M_PI) * 2;
        drawWave(SCREEN_HEIGHT - 8, _spd, _amp);

        showDisplay();
    }

    cyw43_arch_deinit();
}
