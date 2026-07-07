#include <util/delay.h>
#include "hardware.h"   // korzysta z Twojego HW_ShiftOut, nie zmienia go
#include "arrow.h"

#define GROT_L   0b00100100
#define TRZON    0b01011010
#define GROT_P   0b10000001

#define STRZALKA_L (GROT_L | TRZON)
#define STRZALKA_P (GROT_P | TRZON)

static void blink(uint8_t maska, uint8_t razy)
{
    while (razy--)
    {
        HW_ShiftOut(maska);
        _delay_ms(300);
        HW_ShiftOut(0);
        _delay_ms(300);
    }
}

void Arrow_Left(uint8_t razy)  { blink(STRZALKA_L, razy); }
void Arrow_Right(uint8_t razy) { blink(STRZALKA_P, razy); }