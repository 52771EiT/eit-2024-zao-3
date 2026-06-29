/*
 * I2C_Scanner - pomocniczy skecz do znalezienia adresu wyswietlacza LCD.
 *
 * Jak uzyc:
 *   1. Otworz TEN plik osobno w Arduino IDE i wgraj na plytke.
 *   2. Otworz Monitor portu szeregowego (9600 baud).
 *   3. Odczytaj wykryty adres (np. 0x27 albo 0x3F).
 *   4. Wpisz ten adres w konstruktorze DisplayManager w glownym skeczu.
 *
 * Po znalezieniu adresu ten plik nie jest juz potrzebny w dzialaniu systemu.
 */

#include <Wire.h>

void setup() {
    Wire.begin();
    Serial.begin(9600);
    while (!Serial) { }
    Serial.println(F("Skaner I2C - szukam urzadzen..."));
}

void loop() {
    byte count = 0;
    for (byte addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.print(F("Znaleziono urzadzenie I2C pod adresem 0x"));
            if (addr < 16) Serial.print('0');
            Serial.println(addr, HEX);
            count++;
        }
    }
    if (count == 0) Serial.println(F("Brak urzadzen - sprawdz podlaczenie SDA(A4)/SCL(A5)."));
    Serial.println(F("---"));
    delay(3000);
}
