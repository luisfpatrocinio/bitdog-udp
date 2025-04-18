#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/watchdog.h"

// Patro libs
#include "display.h"
#include "draw.h"
#include "text.h"
#include "buttons.h"
#include "led.h"
#include "wifi_udp.h"
#include "approach.h"

// Math
#include "math.h"

// Time
#include "time.h"

// Global variables
int pressedTimer = 0;
float sinAmplitude = 1.0;

void runCommand(char *msg);
void runCommandParam(char *msg, char *param);

void runCommand(char *msg)
{
    char *param = strchr(msg, ' ');
    if (param != NULL)
    {
        *param = '\0'; // Split the command and parameter
        param++;       // Move to the parameter
    }
    else
    {
        param = ""; // No parameter provided
    }

    runCommandParam(msg, param);
}

void runCommandParam(char *msg, char *param)
{
    // PING
    if (strcmp(msg, "ping") == 0)
    {
        printf("[UDP] PING -> Respondendo com PONG'\n");
        sendUDP("pong");
    }

    // RESET
    else if (strcmp(msg, "reset") == 0)
    {
        printf("[UDP] RESET solicitado.");
        watchdog_enable(1, 1);
        while (true)
        {
            // Wait for the watchdog to reset the device
            tight_loop_contents();
        }
    }

    // CONNECTED
    else if (strcmp(msg, "connected") == 0)
    {
        printf("[UDP] Connected to %s:%d\n", ipaddr_ntoa(&gTargetIP), UDP_PORT);
    }

    // TURN LED RED
    else if (strcmp(msg, "turn_led_red") == 0)
    {
        if (strcmp(param, "on") == 0)
        {
            printf("[UDP] LED RED ON\n");
            setLedBrightness(LED_RED_PIN, 255);
        }
        else if (strcmp(param, "off") == 0)
        {
            printf("[UDP] LED RED OFF\n");
            setLedBrightness(LED_RED_PIN, 0);
        }
        else
        {
            printf("Unknown parameter for turn_led_red: %s\n", param);
        }
    }

    // TURN LED BLUE
    else if (strcmp(msg, "turn_led_blue") == 0)
    {
        if (strcmp(param, "on") == 0)
        {
            printf("[UDP] LED BLUE ON\n");
            setLedBrightness(LED_BLUE_PIN, 255);
        }
        else if (strcmp(param, "off") == 0)
        {
            printf("[UDP] LED BLUE OFF\n");
            setLedBrightness(LED_BLUE_PIN, 0);
        }
        else
        {
            printf("Unknown parameter for turn_led_blue: %s\n", param);
        }
    }

    // TURN LED GREEN
    else if (strcmp(msg, "turn_led_green") == 0)
    {
        if (strcmp(param, "on") == 0)
        {
            printf("[UDP] LED GREEN ON\n");
            setLedBrightness(LED_GREEN_PIN, 255);
        }
        else if (strcmp(param, "off") == 0)
        {
            printf("[UDP] LED GREEN OFF\n");
            setLedBrightness(LED_GREEN_PIN, 0);
        }
        else
        {
            printf("Unknown parameter for turn_led_green: %s\n", param);
        }
    }

    else if (strcmp(msg, "set_led_red_brightness") == 0)
    {
        int brightness = atoi(param); // Convert the parameter to an integer
        if (brightness >= 0 && brightness <= 255)
        {
            printf("[UDP] Setting RED LED brightness to %d\n", brightness);
            setLedBrightness(LED_RED_PIN, brightness);
        }
        else
        {
            printf("Invalid brightness value for RED LED: %s. Must be between 0 and 255.\n", param);
        }
    }
    else if (strcmp(msg, "set_led_blue_brightness") == 0)
    {
        int brightness = atoi(param); // Convert the parameter to an integer
        if (brightness >= 0 && brightness <= 255)
        {
            printf("[UDP] Setting BLUE LED brightness to %d\n", brightness);
            setLedBrightness(LED_BLUE_PIN, brightness);
        }
        else
        {
            printf("Invalid brightness value for BLUE LED: %s. Must be between 0 and 255.\n", param);
        }
    }
    else if (strcmp(msg, "set_led_green_brightness") == 0)
    {
        int brightness = atoi(param); // Convert the parameter to an integer
        if (brightness >= 0 && brightness <= 255)
        {
            printf("[UDP] Setting GREEN LED brightness to %d\n", brightness);
            setLedBrightness(LED_GREEN_PIN, brightness);
        }
        else
        {
            printf("Invalid brightness value for GREEN LED: %s. Must be between 0 and 255.\n", param);
        }
    }
    else
    {
        printf("Unknown command: %s\n", msg);
    }
}

/* -- Callback Functions -- */
void buttonCallback(uint gpio, uint32_t events)
{
    if (pressedTimer > 0)
        return; // Ignore button presses while timer is active

    if (gpio == BTA && isButtonPressed(BTA))
    {
        pressedTimer = 10;
        printf("Button A pressed\n");
        sendUDP("Hello from Patro!");
    }
    else if (gpio == BTB && isButtonPressed(BTB))
    {
        pressedTimer = 10;
        printf("Button B pressed\n");
        sendUDP("Patro te leva de nave.");
    }
}

bool timerCallback(struct repeating_timer *t)
{
    static int counter = 0;
    char msg[4];
    snprintf(msg, sizeof(msg), "%d", counter);
    sendUDP(msg);
    counter++;
    counter %= 100; // Reset counter every 1000 iterations
    return true;    // Keep the timer running
}

void udpReceiveCallback(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    if (!p)
    {
        printf("Received empty packet\n");
        return;
    }

    // Atualizar gTargetIP
    gTargetIP = *addr;

    // Copia os dados recebidos para uma string (garantindo null terminator)
    char msg[BEACON_MSG_LEN_MAX + 1];
    memset(msg, 0, sizeof(msg));
    memcpy(msg, p->payload, MIN(p->len, BEACON_MSG_LEN_MAX));

    printf("Received UDP packet from %s:%d:\n", ipaddr_ntoa(addr), port);

    sinAmplitude = 4 + (rand() % 5); // Random amplitude between 2 and 6

    // Manage Received
    runCommand(msg);

    pbuf_free(p);
}
/* ----- */

void setup()
{
    stdio_init_all();
    initI2C();
    initDisplay();
    initButtons();
    setButtonCallback(buttonCallback);
    initLeds();
    setAllLedsBrightness(0);

    clearDisplay();
    drawTextCentered("Patro UDP", 16);
    drawTextCentered("Starting...", 24);
    showDisplay();
}

void drawInterface()
{
    // Draw Header
    drawTextCentered("BitDog Patro Station", 2);

    // if has ip
    if (gTargetIP.addr != 0)
    {
        // Draw IP
        char ipStr[16];
        snprintf(ipStr, sizeof(ipStr), "IP: %s", ipaddr_ntoa(&gTargetIP));
        drawText(0, 10, ipStr);

        // Draw Port
        char portStr[16];
        snprintf(portStr, sizeof(portStr), "Port: %d", UDP_PORT);
        drawText(0, 18, portStr);
    }
    else
    {
        drawTextCentered("No Connection", 10);
        drawTextCentered("Open PatroStation", 18);
        drawTextCentered("on the same network", 26);
    }

    // Draw Wave
    sinAmplitude = approach(sinAmplitude, 1, 0.05f); // Smoothly approach to 1
    int _spd = 18;
    float t = time_us_32() / 1e6;
    int _amp = sinAmplitude + sinf(t * 2 * M_PI) * 2;
    drawWave(SCREEN_HEIGHT - 8, _spd, _amp);
}

int main()
{
    // Setup
    setup();
    wifiSetup();

    // Create a UDP PCB (Protocol Control Block)
    gPCB = udp_new();
    if (!gPCB)
    {
        printf("Failed to create UDP PCB\n");
        drawError("PCB fail");
        return 1;
    }

    // Set up the UDP receive callback to handle incoming packets
    udp_recv(gPCB, udpReceiveCallback, NULL);

    // Add a repeating timer to send UDP packets
    add_repeating_timer_ms(69, timerCallback, NULL, &sendUDPTimer);

    while (true)
    {
        // Button press cooldown
        pressedTimer = MAX(pressedTimer - 1, 0);

        clearDisplay();

        drawInterface();

        showDisplay();
    }
}
