#include "DisplayManager.h"

DisplayManager::DisplayManager(uint8_t i2cAddress, uint8_t cols, uint8_t rows)
    : _lcd(i2cAddress, cols, rows), _cols(cols), _rows(rows), _count(0) {}

void DisplayManager::begin() {
    _lcd.init();        // niektore wersje biblioteki uzywaja begin() zamiast init()
    _lcd.backlight();
    _lcd.clear();
}

void DisplayManager::clear() {
    _lcd.clear();
}

void DisplayManager::clearRow(uint8_t row) {
    _lcd.setCursor(0, row);
    for (uint8_t i = 0; i < _cols; i++) _lcd.print(' ');
    _lcd.setCursor(0, row);
}

void DisplayManager::printCentered(uint8_t row, const String &text) {
    clearRow(row);
    int pad = ((int)_cols - (int)text.length()) / 2;
    if (pad < 0) pad = 0;
    _lcd.setCursor((uint8_t)pad, row);
    _lcd.print(text);
}

void DisplayManager::updateCounter(int count) {
    _count = count;
    _lcd.clear();
    printCentered(0, String("Osoby: ") + String(count));
}

void DisplayManager::showMessage(const String &line1) {
    _lcd.clear();
    printCentered(0, line1);
}

void DisplayManager::showMessage(const String &line1, const String &line2) {
    _lcd.clear();
    printCentered(0, line1);
    printCentered(1, line2);
}
