#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "SystemState.h"
#include "../Led/LedController.h"
#include "../Display/DisplayManager.h"
#include "../Buzzer/Buzzer.h"

/*
 * UiController - KOORDYNATOR warstwy sygnalizacji.
 *
 * Jedna metoda setState(stan) ustawia naraz: komunikat na LCD, wzorzec diod
 * LED oraz sygnal buzzera - dokladnie wg tabeli stanow ze specyfikacji.
 *
 * Tego uzywa zespol logiki. Przyklad:
 *     ui.setCount(licznik);                 // gdy zmienia sie liczba osob
 *     ui.setState(SystemState::ENTER);      // gdy zaczyna sie wejscie
 * a w loop():
 *     ui.update();                          // naped LED + buzzera
 *
 * Buzzer jest opcjonalny - przekaz nullptr (lub pomin), jesli go nie uzywasz.
 */

class UiController {
public:
    UiController(LedController& leds, DisplayManager& lcd, Buzzer* buzzer = nullptr);

    void begin();                 // ustawia stan poczatkowy IDLE
    void update();                // wolaj w loop()

    void setState(SystemState s); // LCD + LED + buzzer wg tabeli
    void setCount(int count);     // aktualizacja licznika (odswieza ekran IDLE)

    SystemState state() const { return _state; }
    int         count() const { return _count; }

private:
    LedController&  _leds;
    DisplayManager& _lcd;
    Buzzer*         _buzzer;
    SystemState     _state;
    int             _count;

    void beepShort(uint8_t n);
    void beepLong();
    void beepReset();
};

#endif // UI_CONTROLLER_H
