/*
 * ============================================================================
 *  eit-2024-zao-3 - SYSTEM KONTROLI ILOSCI OSOB NA OBIEKCIE
 *  MODUL: LED + WYSWIETLACZ LCD (+ buzzer)
 * ============================================================================
 *
 *  SZKIELET / DEMO. Po wgraniu na plytke leci automatyczna prezentacja
 *  WSZYSTKICH stanow ze specyfikacji (IDLE, ENTER, ENTER_PASSING, LOTTERY,
 *  EXIT, EXIT_PASSING, SEARCH, LOCK, ERROR, RESET) - mozesz sprawdzic LCD,
 *  diody i buzzer na biurku BEZ kodu reszty zespolu.
 *
 *  Zespol od logiki/drzwi/losomatu podmienia tylko cialo loop() na realne
 *  zdarzenia (przyciski, krancowki, RFID), wolajac te same metody:
 *      ui.setCount(n);
 *      ui.setState(SystemState::ENTER);
 *
 *  ---------------------------------------------------------------------------
 *  SPRZET (wg projekt MIKRO.sch, Arduino Nano):
 *    LED zielona "wejscie" (->) -> A0
 *    LED zielona "wyjscie" (<-) -> A1
 *    LED czerwona "X"           -> A2
 *    LED zolta "losowanie"      -> A3
 *    LCD I2C: SDA -> A4, SCL -> A5
 *    Buzzer/glosnik SPK1        -> D8
 *
 *  WYMAGANA BIBLIOTEKA (Menedzer bibliotek Arduino):
 *    "LiquidCrystal I2C"  (autor: Frank de Brabander)
 *  ---------------------------------------------------------------------------
 */

#include "src/Led/LedController.h"
#include "src/Display/DisplayManager.h"
#include "src/Buzzer/Buzzer.h"
#include "src/Ui/UiController.h"

LedController  leds;                  // A0..A3, active-HIGH (zmien na false jesli diody odwrotnie)
DisplayManager lcd(0x27, 16, 2);      // adres I2C 0x27, ekran 16x2 (gdy nie pisze -> 0x3F)
Buzzer         buzzer(8, 2500);       // SPK1 na D8. Jesli buzzer to nie Twoja dzialka:
                                      //   usun te linie i daj nullptr zamiast &buzzer ponizej.
UiController   ui(leds, lcd, &buzzer);

int people = 0;

// Kolejnosc demonstracji wszystkich stanow:
SystemState demo[] = {
    SystemState::IDLE, SystemState::ENTER, SystemState::ENTER_PASSING,
    SystemState::LOTTERY, SystemState::EXIT, SystemState::EXIT_PASSING,
    SystemState::SEARCH, SystemState::LOCK, SystemState::ERROR, SystemState::RESET
};
const uint8_t DEMO_LEN = sizeof(demo) / sizeof(demo[0]);

uint8_t  demoIndex = 0;
uint32_t stepStart = 0;
const uint32_t STEP_MS = 2600;   // ile trwa pokaz jednego stanu

void setup() {
    leds.begin();
    lcd.begin();
    buzzer.begin();
    ui.begin();              // stan poczatkowy IDLE
    ui.setCount(people);
    stepStart = millis();
}

void loop() {
    // >>> NAJWAZNIEJSZE: update() musi byc wolane caly czas <<<
    ui.update();

    // --- ponizej tylko DEMO; w wersji finalnej zastap obsluga zdarzen ---
    if (millis() - stepStart < STEP_MS) return;
    stepStart = millis();

    SystemState s = demo[demoIndex];

    // proste symulowanie licznika:
    if (s == SystemState::ENTER_PASSING) people++;             // ktos wszedl
    if (s == SystemState::EXIT_PASSING && people > 0) people--; // ktos wyszedl

    ui.setCount(people);   // zsynchronizuj licznik z UI
    ui.setState(s);        // ustaw stan: LCD + LED + buzzer
    if (s == SystemState::RESET) people = 0;

    demoIndex = (demoIndex + 1) % DEMO_LEN;
}
