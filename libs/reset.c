#include "reset.h"
void resetProgram()
{
    watchdog_enable(1, 1);
    while (true)
    {
        // Wait for the watchdog to reset the device
        tight_loop_contents();
    }
}