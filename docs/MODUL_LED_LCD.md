# Moduł LED + Wyświetlacz LCD (+ buzzer)

Część projektu **„System kontroli ilości osób na obiekcie"** odpowiadająca za
komunikację wizualną i dźwiękową: diody LED, wyświetlacz LCD i buzzer.

Kod jest **obiektowy** i **nieblokujący** (zero `delay()`). Reszta zespołu
woła jedną metodę — `ui.setState(...)` — a moduł sam ustawia LCD, diody i
buzzer zgodnie z tabelą stanów.

---

## Mapa podłączeń (wg `projekt MIKRO.sch`, Arduino Nano)

| Element                     | Pin Arduino |
|-----------------------------|-------------|
| LED zielona „wejście" (→)   | A0          |
| LED zielona „wyjście" (←)   | A1          |
| LED czerwona „X / blokada"  | A2          |
| LED żółta „losowanie"       | A3          |
| LCD I2C — SDA               | A4          |
| LCD I2C — SCL               | A5          |
| Buzzer / głośnik SPK1       | D8          |

> **Polaryzacja diod:** kod domyślnie zakłada `active-HIGH`. Jeśli na płytce
> diody świecą odwrotnie, w `LedController(...)` ustaw ostatni argument na
> `false`.
>
> **Polskie znaki:** LCD (HD44780) ich nie ma, więc komunikaty są w ASCII
> (bez ogonków) — „Prosze wejsc" zamiast „Proszę wejść".
>
> **„Płynąca strzałka":** schemat ma pojedynczą diodę na kierunek, więc
> „płynąca" = miganie (FLOW), „świeci ciągle" = ON. Pasek na 74HC595 wymagałby
> tylko podmiany wnętrza `LedController` — reszta projektu bez zmian.

---

## Tabela stanów (zaimplementowana w `UiController::setState`)

| Stan            | LCD                        | LED                      | Buzzer        |
|-----------------|----------------------------|--------------------------|---------------|
| `IDLE`          | `Osoby: N`                 | X świeci ciągle          | —             |
| `ENTER`         | `Prosze wejsc`             | → płynie (miga)          | 1 krótki      |
| `ENTER_PASSING` | `Zamknij drzwi`            | → świeci ciągle          | 1 krótki      |
| `LOTTERY`       | `Kliknij losomat`          | X świeci ciągle          | 1 krótki      |
| `EXIT`          | `Prosze wyjsc`             | ← płynie (miga)          | 1 krótki      |
| `EXIT_PASSING`  | `Zamknij drzwi`            | ← świeci ciągle          | 1 krótki      |
| `SEARCH`        | `Kontrola! / Prosze czekac`| X szybko miga            | 3 krótkie     |
| `LOCK`          | `Najpierw uzyj / przycisku`| X szybko miga            | 2 krótkie     |
| `ERROR`         | `Niepoprawna / akcja`      | X bardzo szybko miga     | 1 długi       |
| `RESET`         | `Licznik / wyzerowany`     | wszystkie LED na chwilę  | krótka sekwencja |

---

## Struktura plików

```
/eit-2024-zao-3.ino          <- główny szkielet/demo (przechodzi przez wszystkie stany)
/src
  /Ui
     SystemState.h            <- enum stanów (wspólny z zespołem logiki)
     UiController.h / .cpp    <- KOORDYNATOR: setState() = LCD + LED + buzzer
  /Led
     LedController.h / .cpp   <- sterownik 4 diod (nieblokujący)
  /Display
     DisplayManager.h / .cpp  <- sterownik LCD I2C
  /Buzzer
     Buzzer.h / .cpp          <- sygnały dźwiękowe (OPCJONALNY)
  /Logic                      <- folder zespołu od logiki
  /Hardware                   <- folder zespołu od sprzętu
/examples/LedLcdDemo
     I2C_Scanner.ino          <- skaner adresu I2C wyświetlacza
/docs
     MODUL_LED_LCD.md         <- ten plik
```

---

## Wymagana biblioteka

Arduino IDE → **Narzędzia → Zarządzaj bibliotekami…** → zainstaluj
**„LiquidCrystal I2C"** (autor *Frank de Brabander*). `Wire` i `tone()` są
wbudowane w Arduino.

---

## Jak używa tego zespół od logiki

```cpp
#include "src/Led/LedController.h"
#include "src/Display/DisplayManager.h"
#include "src/Buzzer/Buzzer.h"
#include "src/Ui/UiController.h"

LedController  leds;                 // A0..A3
DisplayManager lcd(0x27, 16, 2);     // adres I2C, 16x2
Buzzer         buzzer(8, 2500);      // D8  (opcjonalnie)
UiController   ui(leds, lcd, &buzzer);

void setup() {
    leds.begin(); lcd.begin(); buzzer.begin();
    ui.begin();                      // stan startowy IDLE
}

void loop() {
    ui.update();                     // ZAWSZE w loop() (napędza LED + buzzer)

    // przykładowe wywołania z logiki:
    // ui.setCount(liczbaOsob);            // gdy zmienia się licznik
    // ui.setState(SystemState::ENTER);    // start procedury wejścia
    // ui.setState(SystemState::SEARCH);   // losomat wytypował do kontroli
    // ui.setState(SystemState::LOCK);     // drzwi bez rozpoczęcia procedury
    // ui.setState(SystemState::RESET);    // przycisk reset
}
```

Buzzer jest opcjonalny — jeśli to działka innej osoby, przekaż `nullptr`
zamiast `&buzzer` (lub w ogóle pomiń ten argument): `UiController ui(leds, lcd);`.

---

## Uruchomienie / test na biurku

1. Zainstaluj bibliotekę **LiquidCrystal I2C**.
2. (Opcjonalnie) wgraj `examples/LedLcdDemo/I2C_Scanner.ino`, by poznać adres
   LCD (zwykle `0x27` lub `0x3F`), i wpisz go w `DisplayManager(...)`.
3. Otwórz `eit-2024-zao-3.ino`, wybierz płytkę **Arduino Nano** i port, wgraj.
4. Poleci automatyczne demo wszystkich stanów — sprawdzisz LCD, diody i buzzer
   bez kodu reszty zespołu. Sekcja demo w `loop()` jest tymczasowa (oznaczona
   w komentarzu); zespół integracyjny zastąpi ją obsługą realnych zdarzeń.
