# eit-2024-zao-3

## System kontroli ilości osób na obiekcie (Arduino Nano)

Projekt zespołowy. Podział na moduły w `src/`:

- `src/Ui`, `src/Led`, `src/Display`, `src/Buzzer` — **moduł LED + LCD + buzzer** (gotowy).
  Sterowanie przez jedną metodę `ui.setState(...)` wg tabeli stanów.
  Dokumentacja i API: [`docs/MODUL_LED_LCD.md`](docs/MODUL_LED_LCD.md).
- `src/Logic` — logika główna i licznik osób (zespół Logika).
- `src/Hardware` — schemat, montaż, zasilanie (zespół Hardware).

Główny szkielet spinający: [`eit-2024-zao-3.ino`](eit-2024-zao-3.ino).
