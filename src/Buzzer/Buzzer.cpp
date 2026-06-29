#include "Buzzer.h"

Buzzer::Buzzer(uint8_t pin, uint16_t freq)
    : _pin(pin), _freq(freq), _count(0), _idx(0), _stepStart(0), _active(false) {}

void Buzzer::begin() {
    pinMode(_pin, OUTPUT);
    noTone(_pin);
}

void Buzzer::load(const uint16_t* durations, const bool* tones, uint8_t n) {
    if (n > MAX_STEPS) n = MAX_STEPS;
    for (uint8_t i = 0; i < n; i++) {
        _dur[i]  = durations[i];
        _tone[i] = tones[i];
    }
    _count     = n;
    _idx       = 0;
    _active    = (n > 0);
    _stepStart = millis();
    if (_active && _tone[0]) tone(_pin, _freq);
    else                     noTone(_pin);
}

void Buzzer::update() {
    if (!_active) return;
    if (millis() - _stepStart < _dur[_idx]) return;   // krok jeszcze trwa

    _idx++;
    if (_idx >= _count) {           // koniec sekwencji
        noTone(_pin);
        _active = false;
        return;
    }
    _stepStart = millis();
    if (_tone[_idx]) tone(_pin, _freq);
    else             noTone(_pin);
}

void Buzzer::beepShort(uint8_t times) {
    if (times < 1) times = 1;
    uint16_t d[MAX_STEPS];
    bool     t[MAX_STEPS];
    uint8_t  n = 0;
    for (uint8_t i = 0; i < times && n < MAX_STEPS - 1; i++) {
        d[n] = 80; t[n] = true;  n++;          // krotki dzwiek
        if (i < times - 1) { d[n] = 80; t[n] = false; n++; }  // przerwa
    }
    load(d, t, n);
}

void Buzzer::beepLong() {
    uint16_t d[1] = { 600 };
    bool     t[1] = { true };
    load(d, t, 1);
}

void Buzzer::resetSequence() {
    uint16_t d[5] = { 60, 40, 60, 40, 140 };
    bool     t[5] = { true, false, true, false, true };
    load(d, t, 5);
}

void Buzzer::stop() {
    noTone(_pin);
    _active = false;
}
