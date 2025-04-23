#include <stdio.h>
#include "pico/stdlib.h"

// Patro libs
#include "display.h"
#include "draw.h"
#include "text.h"
#include "buttons.h"
#include "led.h"
#include "wifi_udp.h"
#include "approach.h"
#include "reset.h"
#include "analog.h"
#include "buzzer.h"

// Math
#include "math.h"

// Time
#include "time.h"

// Global variables
int pressedTimer = 0;
float sinAmplitude = 1.0;
float sinSpeed = 1.0;

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
        resetProgram();
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
    else if (strcmp(msg, "play_tone") == 0)
    {
        int frequency = atoi(param); // Convert the parameter to an integer
        if (frequency > 0)
        {
            printf("[UDP] Playing tone with frequency %d Hz\n", frequency);
            playTone(frequency, 100);
        }
        else
        {
            printf("Invalid frequency value for play_tone: %s. Must be greater than 0.\n", param);
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
        sendUDP("B|0");
    }
    else if (gpio == BTB && isButtonPressed(BTB))
    {
        pressedTimer = 10;
        printf("Button B pressed\n");
        sendUDP("B|1");
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
    initAnalog();
    initBuzzerPWM();

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
        char ipStr[32];
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
    int _amp = sinAmplitude;
    drawWave(SCREEN_HEIGHT - 8, _spd, _amp);

    // Draw analog
    char xStr[16];
    snprintf(xStr, sizeof(xStr), "X: %d", analog_x);
    drawText(0, SCREEN_HEIGHT - 16, xStr);
    char yStr[16];
    snprintf(yStr, sizeof(yStr), "Y: %d", analog_y);
    drawText(0, SCREEN_HEIGHT - 24, yStr);
}

void showConnectingScreen()
{
    static int retryTimer = 0;
    clearDisplay();

    int linkY = SCREEN_HEIGHT - 8;
    switch (wifiGetStatus())
    {
    case CYW43_LINK_DOWN:
        drawTextCentered("Link Down", linkY);
        break;
    case CYW43_LINK_JOIN:
        sinSpeed = approach(sinSpeed, 2, 0.05f);
        sinAmplitude = approach(sinAmplitude, 4, 0.05f);
        drawTextCentered("Link Join", linkY);
        break;
    case CYW43_LINK_NOIP:
        drawTextCentered("Link NoIP", linkY);
        break;
    case CYW43_LINK_UP:
        drawTextCentered("Link UP", linkY);
        break;
    case CYW43_LINK_FAIL:
        drawTextCentered("Link Fail", linkY);
        if (retryTimer <= 0)
            retryTimer = 80;
        break;
    case CYW43_LINK_NONET:
        sinSpeed = approach(sinSpeed, 1, 0.05f);
        sinAmplitude = approach(sinAmplitude, 0, 0.2f);
        drawTextCentered("Link NoNet", linkY);
        if (retryTimer <= 0)
            retryTimer = 80;
        break;
    case CYW43_LINK_BADAUTH:
        drawTextCentered("Link BadAuth", linkY);
        break;
    default:
        drawTextCentered("Link Unknown", linkY);
        break;
    }

    drawTextCentered("Connecting to", 8);
    drawTextCentered(WIFI_SSID, 16);

    if (retryTimer > 0)
    {
        char retryTimerText[20];
        snprintf(retryTimerText, sizeof(retryTimerText), "Retry in %d", retryTimer / 10);
        drawTextCentered(retryTimerText, 24);
        retryTimer -= 1;
        if (retryTimer <= 0)
        {
            wifiConnectAsync(WIFI_SSID, WIFI_PASSWORD);
            printf("Retrying connection...\n");
        }
    }
    else
    {
        static int dots = 0;
        dots = (time_us_32() / 500000) % 3;
        char waitStr[20];
        snprintf(waitStr, sizeof(waitStr), "Please wait%s", dots == 0 ? "." : (dots == 1 ? ".." : "..."));
        drawTextCentered(waitStr, 24);
    }
    drawWave(SCREEN_HEIGHT - 16, sinSpeed, sinAmplitude);
    showDisplay();
}

int main()
{
    // Setup
    setup();
    wifiSetup();

    // Wi-fi connection screen
    wifiConnectAsync(WIFI_SSID, WIFI_PASSWORD);
    while (!wifiIsConnected())
    {
        showConnectingScreen();
    }

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
    // DEATIVADO POR ENQUANTO.
    // add_repeating_timer_ms(69, timerCallback, NULL, &sendUDPTimer);

    while (true)
    {
        // Button press cooldown
        pressedTimer = MAX(pressedTimer - 1, 0);

        clearDisplay();

        // Get Analog
        updateAxis();

        // Send Analog data via udp
        char udpMsg[32];
        snprintf(udpMsg, sizeof(udpMsg), "A|%d|%d", analog_x, analog_y);
        sendUDP(udpMsg);

        // Send Button data via udp
        if (isButtonPressed(BTA))
        {
            sendUDP("B|0");
        }
        if (isButtonPressed(BTB))
        {
            sendUDP("B|1");
        }

        drawInterface();

        showDisplay();
    }
}
