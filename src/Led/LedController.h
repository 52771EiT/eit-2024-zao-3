#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>

/*
 * LedController - sterowanie 4 sygnalizacyjnymi diodami LED.
 *
 * Podlaczenie wg schematu "projekt MIKRO.sch" (Arduino Nano):
 *   LED1  ZIELONA  "wejscie" (strzalka ->)  -> A0
 *   LED2  ZIELONA  "wyjscie" (strzalka <-)  -> A1
 *   LED3  CZERWONA "X / blokada"            -> A2
 *   LED4  ZOLTA    "losowanie"              -> A3   (rezerwa, uzyta w RESET)
 *
 * Modul jest NIEBLOKUJACY: miganie realizuje update(), ktore trzeba wolac
 * w KAZDEJ iteracji loop() (zadne delay()).
 *
 * UWAGA - "plynaca strzalka": schemat ma POJEDYNCZA diode na kierunek,
 * wiec "plynaca strzalka" = miganie tej diody (FLOW), a "swieci ciagle" = ON.
 * Gdy kiedys dojdzie pasek diod na 74HC595, podmienia sie tylko ten plik.
 */

enum class LedPattern : uint8_t {
    OFF,              // zgaszona
    ON,               // swieci ciagle        ("strzalka swieci ciagle", "X swieci ciagle")
    FLOW,             // lagodne miganie ~400 ms  ("plynaca strzalka")
    BLINK_FAST,       // szybkie miganie ~160 ms  ("X szybko miga")
    BLINK_VERY_FAST   // bardzo szybkie ~70 ms    ("X bardzo szybko miga")
};

class LedController {
public:
    // activeHigh = true  -> stan WYSOKI zapala diode (pin -> R -> LED -> GND)
    // activeHigh = false -> stan NISKI zapala diode
    // Sprawdz polaryzacje na realnej plytce i ustaw odpowiednio!
    LedController(uint8_t pinEntry  = A0,
                  uint8_t pinExit   = A1,
                  uint8_t pinBlock  = A2,
                  uint8_t pinRandom = A3,
                  bool    activeHigh = true);

    void begin();
    void update();   // wolaj w loop() jak najczesciej

    void setEntry (LedPattern p);   // zielona "wejscie"  (strzalka ->)
    void setExit  (LedPattern p);   // zielona "wyjscie"  (strzalka <-)
    void setBlock (LedPattern p);   // czerwona "X"
    void setRandom(LedPattern p);   // zolta "losowanie"

    void allOff();                      // gasi wszystkie diody
    void clearLeds() { allOff(); }      // alias (zgodnosc wstecz)
    void flashAll(uint16_t ms = 400);   // RESET: wszystkie LED na chwile

private:
    struct Led { uint8_t pin; LedPattern pattern; };
    Led      _entry, _exit, _block, _random;
    bool     _activeHigh;
    uint32_t _flashUntil;   // do ktorej chwili (millis) trwa blysk wszystkich

    bool patternState(LedPattern p, uint32_t now) const;
    void write(uint8_t pin, bool on);
};

#endif // LED_CONTROLLER_H
