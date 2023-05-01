#pragma once
#include <stdint.h>

#define SYS_EVENT_KEYBOARD 1

struct system_event {
    uint8_t type;
    uint32_t val;
};

void system_events_init();

void system_push_event(uint8_t type, uint32_t val);
struct system_event* system_fetch_event();
void system_pop_event();

int system_has_events();

