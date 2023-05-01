#pragma once
#include <stdint.h>
#define PIC_1_CTRL 0x20
#define PIC_2_CTRL 0xA0
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

void kb_init();

void pic_remap();

void pic_mask(int irq);

void pic_unmask(int irq);

void pic_disable();

void pic_sendEndOfInterrupt(int irq);

uint16_t pic_readIrqReqReg();

uint16_t pic_readInServiceReg();
