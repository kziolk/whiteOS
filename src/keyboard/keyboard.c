#include "keyboard.h"
#include "whitelib/system.h"

uint32_t keyboard_scan_key() {
    int state = 0;
    uint32_t result = 0;

    // loop untill key is pressed and released
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
        }
    }
    return result;
}
