#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <stdint.h>   // uint8_t (niezaleznie od kolejnosci include)

/*
 * SystemState - lista stanow systemu (wg specyfikacji).
 * Ten plik dolacza zarowno modul LED+LCD, jak i zespol logiki -
 * dzieki temu obie strony mowia tym samym "jezykiem".
 */
enum class SystemState : uint8_t {
    IDLE,            // oczekiwanie - "Osoby: N",  X swieci
    ENTER,           // start wejscia - "Prosze wejsc", strzalka -> plynie
    ENTER_PASSING,   // drzwi otwarte - "Zamknij drzwi", strzalka -> ciagle
    LOTTERY,         // przed wyjsciem - "Kliknij losomat", X swieci
    EXIT,            // start wyjscia - "Prosze wyjsc", strzalka <- plynie
    EXIT_PASSING,    // drzwi otwarte - "Zamknij drzwi", strzalka <- ciagle
    SEARCH,          // kontrola - "Kontrola! Prosze czekac", X szybko miga
    LOCK,            // drzwi bez procedury - "Najpierw uzyj przycisku", X szybko
    ERROR,           // zla akcja - "Niepoprawna akcja", X bardzo szybko
    RESET            // zerowanie - "Licznik wyzerowany", wszystkie LED na chwile
};

#endif // SYSTEM_STATE_H
