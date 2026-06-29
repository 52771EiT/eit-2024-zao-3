#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>   // Menedzer bibliotek: "LiquidCrystal I2C" (Frank de Brabander)

/*
 * DisplayManager - obsluga wyswietlacza LCD 16x2 na konwerterze I2C.
 *
 * Podlaczenie (Arduino Nano, wg projekt MIKRO.sch):
 *   SDA -> A4,  SCL -> A5,  VCC -> 5V,  GND -> GND
 *
 * Adres I2C to zwykle 0x27 lub 0x3F - jesli ekran swieci, ale nic nie pisze,
 * zmien adres w konstruktorze (uzyj examples/LedLcdDemo/I2C_Scanner.ino).
 *
 * UWAGA: kontroler HD44780 nie ma polskich znakow, dlatego komunikaty sa w
 * czystym ASCII (bez ogonkow), zeby na ekranie nie bylo "krzakow".
 */

class DisplayManager {
public:
    DisplayManager(uint8_t i2cAddress = 0x27, uint8_t cols = 16, uint8_t rows = 2);

    void begin();

    void updateCounter(int count);                            // "Osoby: N"
    void showMessage(const String &line1);                    // 1 wycentrowany wiersz
    void showMessage(const String &line1, const String &line2); // 2 wiersze
    void clear();

    int getCount() const { return _count; }

private:
    LiquidCrystal_I2C _lcd;
    uint8_t _cols;
    uint8_t _rows;
    int     _count;

    void printCentered(uint8_t row, const String &text);
    void clearRow(uint8_t row);
};

#endif // DISPLAY_MANAGER_H
