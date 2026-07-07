#include <avr/io.h>
#include "hardware.h" // Musisz stworzyć też ten plik nagłówkowy
#define DATA_PIN  PB0   // D8  -> pin 14 DS
#define LATCH_PIN PB3   // D11 -> pin 12 STCP
#define CLOCK_PIN PB4   // D12 -> pin 11 SHCP
void HW_Init(void)
{
    // Ustawiamy DATA, LATCH, CLOCK jako wyjścia
    DDRB |= (1 << DATA_PIN) | (1 << LATCH_PIN) | (1 << CLOCK_PIN);

    // Na start wszystko nisko
    PORTB &= ~(1 << DATA_PIN);
    PORTB &= ~(1 << LATCH_PIN);
    PORTB &= ~(1 << CLOCK_PIN);
}

void HW_ShiftOut(uint8_t data)
{
    // LATCH na LOW, żeby układ jeszcze nie pokazywał zmian
    PORTB &= ~(1 << LATCH_PIN);

    for (uint8_t i = 0; i < 8; i++)
    {
        // CLOCK na LOW
        PORTB &= ~(1 << CLOCK_PIN);

        // Wysyłamy bit od najstarszego do najmłodszego
        if (data & (1 << (7 - i)))
        {
            PORTB |= (1 << DATA_PIN);   // DATA = 1
        }
        else
        {
            PORTB &= ~(1 << DATA_PIN);  // DATA = 0
        }

        // Zbocze zegara: 74HC595 zapamiętuje bit
        PORTB |= (1 << CLOCK_PIN);
    }

    // LATCH na HIGH, dane pojawiają się na wyjściach Q0-Q7
    PORTB |= (1 << LATCH_PIN);
}
