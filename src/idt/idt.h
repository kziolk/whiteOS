#pragma once

#include <stdint.h>

void idt_init();
void disable_interrupts();
void enable_interrupts();

void idt_set_handler(int isr_number, void* handler, uint8_t flags);
// handlers
void idt_exception_handler();
void idt_dummy_handler();
void idt_clock_handler();
void idt_keyboard_handler();

void div_by_zero();

int fetchException();
int* idt_get_tick_counter();
int idt_get_tick_counter2();
