#include "LedController.h"

LedController::LedController(uint8_t pinEntry, uint8_t pinExit,
                             uint8_t pinBlock, uint8_t pinRandom,
                             bool activeHigh)
    : _activeHigh(activeHigh), _flashUntil(0) {
    _entry  = { pinEntry,  LedPattern::OFF };
    _exit   = { pinExit,   LedPattern::OFF };
    _block  = { pinBlock,  LedPattern::OFF };
    _random = { pinRandom, LedPattern::OFF };
}

void LedController::write(uint8_t pin, bool on) {
    digitalWrite(pin, (on == _activeHigh) ? HIGH : LOW);
}

void LedController::begin() {
    pinMode(_entry.pin,  OUTPUT);
    pinMode(_exit.pin,   OUTPUT);
    pinMode(_block.pin,  OUTPUT);
    pinMode(_random.pin, OUTPUT);
    allOff();
}

bool LedController::patternState(LedPattern p, uint32_t now) const {
    switch (p) {
        case LedPattern::OFF:             return false;
        case LedPattern::ON:              return true;
        case LedPattern::FLOW:            return (now % 400) < 200;  // ~plynnie
        case LedPattern::BLINK_FAST:      return (now % 160) < 80;   // szybko
        case LedPattern::BLINK_VERY_FAST: return (now % 70)  < 35;   // bardzo szybko
    }
    return false;
}

void LedController::update() {
    uint32_t now = millis();

    // RESET: przez chwile wszystkie diody zapalone
    if (now < _flashUntil) {
        write(_entry.pin,  true);
        write(_exit.pin,   true);
        write(_block.pin,  true);
        write(_random.pin, true);
        return;
    }

    write(_entry.pin,  patternState(_entry.pattern,  now));
    write(_exit.pin,   patternState(_exit.pattern,   now));
    write(_block.pin,  patternState(_block.pattern,  now));
    write(_random.pin, patternState(_random.pattern, now));
}

void LedController::setEntry (LedPattern p) { _entry.pattern  = p; }
void LedController::setExit  (LedPattern p) { _exit.pattern   = p; }
void LedController::setBlock (LedPattern p) { _block.pattern  = p; }
void LedController::setRandom(LedPattern p) { _random.pattern = p; }

void LedController::allOff() {
    _entry.pattern  = LedPattern::OFF;
    _exit.pattern   = LedPattern::OFF;
    _block.pattern  = LedPattern::OFF;
    _random.pattern = LedPattern::OFF;
    write(_entry.pin,  false);
    write(_exit.pin,   false);
    write(_block.pin,  false);
    write(_random.pin, false);
}

void LedController::flashAll(uint16_t ms) {
    _flashUntil = millis() + ms;   // update() zapali wszystko az do tej chwili
}
