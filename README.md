# System kontroli wejścia/wyjścia z losomatem

Projekt przedstawia prosty system kontroli przejścia zbudowany na mikrokontrolerze **ATmega328P / Arduino UNO/Nano**, wyświetlaczu **LCD I2C**, diodach LED sterowanych przez **74HC595**, czterech przyciskach oraz buzzerze.

System obsługuje procedurę wejścia, procedurę wyjścia, losową kontrolę osoby wychodzącej, licznik osób w środku oraz sygnalizację świetlną i dźwiękową.

---

## Funkcje projektu

Projekt realizuje następujące funkcje:

* symulacja karty wejścia,
* symulacja karty wyjścia,
* losomat z szansą 50/50 na kontrolę,
* licznik osób znajdujących się w środku,
* komunikaty na LCD 16x2,
* strzałka wejścia,
* strzałka wyjścia,
* sygnalizacja kontroli wszystkimi diodami,
* buzzer aktywny podczas odblokowania drzwi,
* buzzer przerywany podczas kontroli,
* blokowanie niepoprawnych akcji,
* powrót do aktualnej procedury po błędnej akcji.

---

## Elementy użyte w projekcie

| Element                         | Opis                                             |
| ------------------------------- | ------------------------------------------------ |
| ATmega328P / Arduino UNO / Nano | Główny mikrokontroler                            |
| LCD 16x2 I2C                    | Wyświetlanie stanu systemu                       |
| 74HC595                         | Rejestr przesuwny do sterowania diodami          |
| Diody LED                       | Strzałki oraz sygnalizacja kontroli              |
| 4 przyciski                     | Karta wejścia, karta wyjścia, losomat, krańcówka |
| Buzzer EAK-00786                | Aktywny buzzer 5 V                               |
| Płytka prototypowa              | Połączenia testowe                               |

---

## Podłączenie LCD I2C

Dla Arduino UNO/Nano:

| LCD I2C | Arduino |
| ------- | ------- |
| GND     | GND     |
| VCC     | 5V      |
| SDA     | A4      |
| SCL     | A5      |

W kodzie domyślnie używany jest adres:

```c
LCD_Init(0x27);
```

Jeżeli LCD nie działa, warto sprawdzić adres `0x3F`:

```c
LCD_Init(0x3F);
```

---

## Podłączenie przycisków

Przyciski działają z wewnętrznymi rezystorami pull-up mikrokontrolera.

Każdy przycisk należy podłączyć według schematu:

```text
PIN Arduino ---- przycisk ---- GND
```

Nie podłączamy przycisków do 5 V.

| Funkcja         | Pin Arduino | Port AVR |
| --------------- | ----------: | -------- |
| Karta wejścia   |          D2 | PD2      |
| Karta wyjścia   |          D3 | PD3      |
| Losomat         |          D4 | PD4      |
| Krańcówka drzwi |          D5 | PD5      |

Logika wejść:

```text
przycisk puszczony = HIGH = 1
przycisk wciśnięty = LOW  = 0
```

Dlatego w kodzie używane jest makro:

```c
#define PRESSED(pin) (!(PIND & (1 << pin)))
```

---

## Podłączenie buzzera

Użyty buzzer: **EAK-00786**, czyli aktywny buzzer z generatorem.

Podłączenie:

| Buzzer | Arduino |
| ------ | ------- |
| +      | D6      |
| -      | GND     |

W kodzie:

```c
#define BUZZER_PIN PD6
```

Buzzer działa następująco:

| Stan              | Działanie buzzera          |
| ----------------- | -------------------------- |
| Drzwi odblokowane | sygnał ciągły              |
| Osoba przechodzi  | buzzer wyłączony           |
| Kontrola          | sygnał przerywany co 0,5 s |
| Idle              | buzzer wyłączony           |

---

## Podłączenie rejestru 74HC595

Rejestr 74HC595 służy do sterowania diodami LED, które tworzą strzałki oraz sygnalizację kontroli.

W projekcie wykorzystywana jest funkcja:

```c
HW_ShiftOut(uint8_t data);
```

Przykładowe wartości wysyłane do rejestru:

```c
#define STRZALKA_L  (GROT_L | TRZON)
#define STRZALKA_P  (GROT_P | TRZON)
#define LED_OFF     0x00
#define LED_ALL     0xFF
```

Znaczenie:

| Wartość      | Efekt                              |
| ------------ | ---------------------------------- |
| `STRZALKA_P` | strzałka w prawo, wejście          |
| `STRZALKA_L` | strzałka w lewo, wyjście           |
| `LED_OFF`    | wszystkie diody zgaszone           |
| `LED_ALL`    | wszystkie diody zapalone, kontrola |

---

## Logika działania systemu

### Stan początkowy

Po uruchomieniu system przechodzi do stanu `Idle`.

LCD pokazuje:

```text
Idle
Osoby: 0
```

System czeka na jedną z akcji:

* karta wejścia,
* karta wyjścia,
* losomat,
* krańcówka drzwi.

---

## Procedura wejścia

### 1. Użycie karty wejścia

Użytkownik naciska przycisk karty wejścia.

System przechodzi do stanu:

```c
STATE_ENTRY_UNLOCKED
```

LCD pokazuje:

```text
Prosze wejsc
Czas: 5 sek
```

W tym stanie:

* zapala się strzałka w prawo,
* buzzer piszczy ciągle,
* drzwi są logicznie odblokowane przez 5 sekund.

---

### 2. Brak przejścia

Jeżeli w ciągu 5 sekund krańcówka nie zostanie aktywowana, system blokuje procedurę.

LCD pokazuje:

```text
Czas minal
Zablokowano
```

Następnie system wraca do:

```c
STATE_IDLE
```

---

### 3. Osoba przechodzi

Jeżeli osoba aktywuje krańcówkę, system przechodzi do stanu:

```c
STATE_ENTRY_PASSING
```

LCD pokazuje:

```text
Drzwi otwarte
Przechodzi...
```

W tym momencie:

* strzałka gaśnie,
* buzzer gaśnie.

---

### 4. Zakończenie wejścia

Po puszczeniu krańcówki system uznaje, że osoba weszła.

Licznik zwiększa się o 1:

```text
Osoby: 1
```

System wraca do stanu `Idle`.

---

## Procedura wyjścia

### 1. Użycie karty wyjścia

Użytkownik naciska przycisk karty wyjścia.

Jeżeli licznik osób jest większy od 0, system przechodzi do stanu:

```c
STATE_EXIT_WAIT_LOTTERY
```

LCD pokazuje:

```text
Wcisnij
losomat
```

System czeka na kliknięcie losomatu.

---

### 2. Losowanie

Po naciśnięciu losomatu system losuje wynik 50/50.

Możliwe wyniki:

* brak kontroli,
* kontrola.

---

### 3. Brak kontroli

Jeżeli nie zostanie wylosowana kontrola, system przechodzi do:

```c
STATE_EXIT_UNLOCKED
```

LCD pokazuje:

```text
Prosze wyjsc
Czas: 5 sek
```

W tym stanie:

* zapala się strzałka w lewo,
* buzzer piszczy ciągle,
* użytkownik ma 5 sekund na rozpoczęcie przejścia.

---

### 4. Kontrola

Jeżeli zostanie wylosowana kontrola, system przechodzi do:

```c
STATE_EXIT_SEARCH
```

LCD pokazuje:

```text
Kontrola!
15 sekund
```

W tym stanie:

* świecą wszystkie diody,
* buzzer piszczy przerywanie,
* kontrola trwa 15 sekund.

Buzzer podczas kontroli działa w trybie:

```text
0,5 s ON
0,5 s OFF
0,5 s ON
0,5 s OFF
```

Po 15 sekundach system pozwala na wyjście.

---

### 5. Osoba wychodzi

Po zakończeniu kontroli albo po braku kontroli użytkownik może wyjść.

Po aktywowaniu krańcówki system przechodzi do:

```c
STATE_EXIT_PASSING
```

LCD pokazuje:

```text
Drzwi otwarte
Wychodzi...
```

W tym momencie:

* strzałka gaśnie,
* buzzer gaśnie.

---

### 6. Zakończenie wyjścia

Po puszczeniu krańcówki system uznaje, że osoba wyszła.

Licznik zmniejsza się o 1:

```text
Osoby: 0
```

System wraca do stanu `Idle`.

---

## Niepoprawne akcje

System blokuje błędne akcje, ale nie resetuje całej procedury.

Przykłady:

| Sytuacja                       | Komunikat               |
| ------------------------------ | ----------------------- |
| Losomat wciśnięty bez karty    | `Najpierw / uzyj karty` |
| Karta wyjścia przy liczniku 0  | `Brak osob / w srodku`  |
| Drzwi otwarte przed losomatem  | `Najpierw / losomat`    |
| Drzwi otwarte podczas kontroli | `Trwa / kontrola`       |
| Zła karta w trakcie procedury  | `Niepoprawna / akcja`   |

Po pokazaniu komunikatu system wraca do poprzedniego stanu procedury.

Przykład:

```text
Wcisnij
losomat
```

Jeżeli użytkownik otworzy drzwi przed kliknięciem losomatu:

```text
Najpierw
losomat
```

Po chwili system wraca do:

```text
Wcisnij
losomat
```

---

## Stany systemu

| Stan                      | Znaczenie                       |
| ------------------------- | ------------------------------- |
| `STATE_IDLE`              | Normalna praca, oczekiwanie     |
| `STATE_ENTRY_UNLOCKED`    | Wejście odblokowane na 5 sekund |
| `STATE_ENTRY_PASSING`     | Osoba wchodzi                   |
| `STATE_EXIT_WAIT_LOTTERY` | Oczekiwanie na losomat          |
| `STATE_EXIT_SEARCH`       | Kontrola 15 sekund              |
| `STATE_EXIT_UNLOCKED`     | Wyjście odblokowane na 5 sekund |
| `STATE_EXIT_PASSING`      | Osoba wychodzi                  |

---

## Diagram działania

```text
WEJŚCIE

Idle
  |
  | karta wejścia
  v
Entry Unlocked
  |
  | krańcówka aktywna
  v
Entry Passing
  |
  | krańcówka puszczona
  v
Idle + licznik +1
```

```text
WYJŚCIE

Idle
  |
  | karta wyjścia
  v
Wait Lottery
  |
  | losomat
  v
Kontrola? 50/50
  |
  +---- brak kontroli ----> Exit Unlocked
  |
  +---- kontrola ---------> Search 15 s ----> Exit Unlocked

Exit Unlocked
  |
  | krańcówka aktywna
  v
Exit Passing
  |
  | krańcówka puszczona
  v
Idle + licznik -1
```

---

## Struktura projektu

Przykładowa struktura plików:

```text
AVR/
├── main.c
├── lcd_i2c.c
├── lcd_i2c.h
├── hardware.c
├── hardware.h
├── makefile
└── README.md
```

Opcjonalnie, jeżeli używasz oddzielnego modułu strzałek:

```text
├── arrow.c
└── arrow.h
```

W aktualnej wersji główna logika używa bezpośrednio:

```c
HW_ShiftOut(...);
```

więc `arrow.c` nie jest wymagany.

---

## Kompilacja

Przykładowy fragment `makefile`:

```makefile
SRC = \
main.c \
lcd_i2c.c \
hardware.c

INC = \
-I.
```

Jeżeli zostawiasz `arrow.c`, możesz mieć:

```makefile
SRC = \
main.c \
lcd_i2c.c \
hardware.c \
arrow.c

INC = \
-I.
```

Kompilacja:

```bash
make
```

Wgranie programu zależy od konfiguracji Twojego `makefile`, np.:

```bash
make flash
```

---

## Najczęstsze problemy

### LCD świeci, ale nic nie pokazuje

Sprawdź:

* adres LCD: `0x27` albo `0x3F`,
* potencjometr kontrastu na module I2C,
* przewody SDA/SCL,
* zasilanie 5 V.

---

### Przyciski działają odwrotnie

W tym projekcie przyciski są podłączone do GND.

Poprawne podłączenie:

```text
pin Arduino ---- przycisk ---- GND
```

Nie używamy zewnętrznego rezystora pull-up, ponieważ pull-up jest włączony programowo.

---

### Przycisk cały czas jest wciśnięty

Prawdopodobnie przycisk 4-nóżkowy jest źle włożony w płytkę prototypową.

Najlepiej włożyć go przez środkowy rowek płytki.

Należy znaleźć dwie nóżki, które:

```text
nie przewodzą, gdy przycisk jest puszczony
przewodzą, gdy przycisk jest wciśnięty
```

---

### Buzzer nie piszczy

Sprawdź:

```text
+ buzzera -> D6
- buzzera -> GND
```

Dla buzzera EAK-00786 wystarczy podać stan wysoki na D6, ponieważ jest to buzzer aktywny z generatorem.

---

### Strzałki świecą odwrotnie

Jeżeli kierunki są zamienione, wystarczy zamienić funkcje:

```c
static void Arrow_Entry(void)
{
    HW_ShiftOut(STRZALKA_P);
}

static void Arrow_Exit(void)
{
    HW_ShiftOut(STRZALKA_L);
}
```

na:

```c
static void Arrow_Entry(void)
{
    HW_ShiftOut(STRZALKA_L);
}

static void Arrow_Exit(void)
{
    HW_ShiftOut(STRZALKA_P);
}
```

---

## Autorzy

Projekt przygotowany jako ćwiczenie z programowania mikrokontrolerów AVR, obsługi LCD I2C, rejestru przesuwnego 74HC595 oraz implementacji prostej maszyny stanów w języku C.

---

## Podsumowanie

Projekt pokazuje praktyczne zastosowanie maszyny stanów w systemie kontroli przejścia.

Najważniejsze elementy działania:

* karta wejścia uruchamia wejście,
* karta wyjścia uruchamia wyjście,
* losomat decyduje o kontroli,
* kontrola trwa 15 sekund,
* przejście musi nastąpić w ciągu 5 sekund od odblokowania,
* licznik osób aktualizuje się po puszczeniu krańcówki,
* błędne akcje nie przerywają procedury,
* LCD pokazuje aktualny stan systemu,
* LED-y i buzzer sygnalizują aktualną akcję.
