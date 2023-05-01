#include "system.h"
#include "stddef.h"
#include "terminal/terminal.h"

#define EVENTS_SIZE 100

size_t head, tail;
struct system_event events[EVENTS_SIZE];

void system_events_init() {
    for (int i = 0; i < EVENTS_SIZE; i++) {
        events[i].type = 0;
    }
    head = 0;
    tail = 0;
}

void system_push_event(uint8_t type, uint32_t val) {
    events[tail].type = type;
    events[tail].val = val;
    tail = (tail + 1) % EVENTS_SIZE;
}

struct system_event* system_fetch_event() {
    return &events[head];
}

void system_pop_event() {
    struct system_event* event = &events[head];
    event->type = 0;
    head = (head + 1) % EVENTS_SIZE;
}

int system_has_events() {
    return (events[head].type != 0);
}

