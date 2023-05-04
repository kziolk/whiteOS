#include "keyboard.h"
#include "whitelib/system.h"

// waits for a system event
// if keyboard event appears then return the key value
uint8_t keyboard_scan_key()
{
    int state = 0;
    uint8_t result = 0;

    // loop untill keyboard event appears
    while(state == 0) {
        // fetch system events
        while(system_has_events()) {
            struct system_event* e = system_fetch_event();
            if (e->type == SYS_EVENT_KEYBOARD) {
                // save value on press
                if (state == 0) result = e->val;
                state++;
            }
            system_pop_event();
            // unwanted behaviour:
            // 1. if there are 2 keyboard events at the same time the value of 1st one gets overwritten
            // 2. if before entering the loop system has pending events then this function will
            //      instantly return not waiting for a new keystroke to appear
        }
    }
    return result;
}
