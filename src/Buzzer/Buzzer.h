#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

/*
 * Buzzer - nieblokujaca sygnalizacja dzwiekowa.
 * Wg schematu glosnik/buzzer SPK1 jest na pinie D8.
 *
 * Sekwencje grane sa krok po kroku w update() (zadne delay()), wiec dzwiek
 * nie blokuje reszty systemu. Uzywa funkcji tone()/noTone().
 *
 * MODUL OPCJONALNY: jesli buzzer to dzialka innej osoby, po prostu nie twórz
 * obiektu tej klasy - LED + LCD dzialaja niezaleznie.
 */

class Buzzer {
public:
    Buzzer(uint8_t pin = 8, uint16_t freq = 2500);

    void begin();
    void update();                       // wolaj w loop()

    void beepShort(uint8_t times = 1);   // n krotkich sygnalow
    void beepLong();                     // 1 dlugi sygnal
    void resetSequence();                // krotka sekwencja (RESET)
    void stop();                         // cisza

private:
    static const uint8_t MAX_STEPS = 16;
    uint8_t  _pin;
    uint16_t _freq;
    uint16_t _dur[MAX_STEPS];   // czas trwania kroku [ms]
    bool     _tone[MAX_STEPS];  // czy w tym kroku gra dzwiek
    uint8_t  _count;            // liczba krokow w sekwencji
    uint8_t  _idx;              // biezacy krok
    uint32_t _stepStart;        // millis() poczatku biezacego kroku
    bool     _active;

    void load(const uint16_t* durations, const bool* tones, uint8_t n);
};

#endif // BUZZER_H
