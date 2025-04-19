#ifndef RESET_H
#define RESET_H
#include "pico/stdlib.h" // Include the header for tight_loop_contents
#include "hardware/watchdog.h" // Include the header for watchdog functions
void resetProgram(void);
#endif // RESET_H