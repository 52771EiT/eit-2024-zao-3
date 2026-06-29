#include "UiController.h"

UiController::UiController(LedController& leds, DisplayManager& lcd, Buzzer* buzzer)
    : _leds(leds), _lcd(lcd), _buzzer(buzzer),
      _state(SystemState::IDLE), _count(0) {}

void UiController::begin() {
    setState(SystemState::IDLE);
}

void UiController::update() {
    _leds.update();
    if (_buzzer) _buzzer->update();
}

void UiController::beepShort(uint8_t n) { if (_buzzer) _buzzer->beepShort(n); }
void UiController::beepLong()           { if (_buzzer) _buzzer->beepLong(); }
void UiController::beepReset()          { if (_buzzer) _buzzer->resetSequence(); }

void UiController::setCount(int count) {
    _count = count;
    if (_state == SystemState::IDLE) _lcd.updateCounter(_count);  // odswiez ekran
}

// ==========================================================================
//  TABELA STANOW - tu mapujemy specyfikacje na LCD + LED + buzzer
// ==========================================================================
void UiController::setState(SystemState s) {
    _state = s;
    _leds.allOff();   // czysty start dla kazdego stanu

    switch (s) {
        case SystemState::IDLE:                       // "Osoby: N" | X swieci | brak
            _lcd.updateCounter(_count);
            _leds.setBlock(LedPattern::ON);
            if (_buzzer) _buzzer->stop();
            break;

        case SystemState::ENTER:                      // "Prosze wejsc" | -> plynie | 1 krotki
            _lcd.showMessage("Prosze wejsc");
            _leds.setEntry(LedPattern::FLOW);
            beepShort(1);
            break;

        case SystemState::ENTER_PASSING:              // "Zamknij drzwi" | -> ciagle | krotki
            _lcd.showMessage("Zamknij drzwi");
            _leds.setEntry(LedPattern::ON);
            beepShort(1);
            break;

        case SystemState::LOTTERY:                    // "Kliknij losomat" | X swieci | krotki
            _lcd.showMessage("Kliknij losomat");
            _leds.setBlock(LedPattern::ON);
            beepShort(1);
            break;

        case SystemState::EXIT:                       // "Prosze wyjsc" | <- plynie | 1 krotki
            _lcd.showMessage("Prosze wyjsc");
            _leds.setExit(LedPattern::FLOW);
            beepShort(1);
            break;

        case SystemState::EXIT_PASSING:               // "Zamknij drzwi" | <- ciagle | krotki
            _lcd.showMessage("Zamknij drzwi");
            _leds.setExit(LedPattern::ON);
            beepShort(1);
            break;

        case SystemState::SEARCH:                     // "Kontrola!/Prosze czekac" | X szybko | 3 krotkie
            _lcd.showMessage("Kontrola!", "Prosze czekac");
            _leds.setBlock(LedPattern::BLINK_FAST);
            beepShort(3);
            break;

        case SystemState::LOCK:                       // "Najpierw uzyj/przycisku" | X szybko | 2 krotkie
            _lcd.showMessage("Najpierw uzyj", "przycisku");
            _leds.setBlock(LedPattern::BLINK_FAST);
            beepShort(2);
            break;

        case SystemState::ERROR:                      // "Niepoprawna/akcja" | X bardzo szybko | dlugi
            _lcd.showMessage("Niepoprawna", "akcja");
            _leds.setBlock(LedPattern::BLINK_VERY_FAST);
            beepLong();
            break;

        case SystemState::RESET:                      // "Licznik/wyzerowany" | wszystkie LED | sekwencja
            _count = 0;
            _lcd.showMessage("Licznik", "wyzerowany");
            _leds.flashAll(400);   // diody juz sa OFF (allOff wyzej) - blysk i gasna
            beepReset();
            break;
    }
}
