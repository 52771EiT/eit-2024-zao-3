#ifndef HARDWARE_H
#define HARDWARE_H

#include <avr/io.h>

void HW_Init(void);
void HW_ShiftOut(uint8_t data);

#endif