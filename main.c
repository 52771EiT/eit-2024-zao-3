#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd_i2c.h"
#include "hardware.h"

// ======================================================
// STRZALKI / LED przez 74HC595
// ======================================================

#define GROT_L   0b00100100
#define TRZON    0b01011010
#define GROT_P   0b10000001

#define STRZALKA_L  (GROT_L | TRZON)
#define STRZALKA_P  (GROT_P | TRZON)

#define LED_OFF     0x00
#define LED_ALL     0xFF   // kontrola = wszystkie diody

// ======================================================
// WEJSCIA / WYJSCIA
// ======================================================

#define BTN_ENTRY_CARD   PD2   // D2 - karta wejscia
#define BTN_EXIT_CARD    PD3   // D3 - karta wyjscia
#define BTN_LOTTERY      PD4   // D4 - losomat
#define DOOR_SENSOR      PD5   // D5 - krancowka drzwi

#define BUZZER_PIN       PD6   // D6 - buzzer aktywny EAK-00786

// Przyciski i krancowka sa do GND, wiec wcisniety = 0
#define PRESSED(pin) (!(PIND & (1 << pin)))

// ======================================================
// CZASY
// ======================================================

#define LOOP_DELAY_MS         20

// 5 sekund = 250 * 20 ms
#define ENTRY_UNLOCK_TIME     250
#define EXIT_UNLOCK_TIME      250

// 15 sekund = 750 * 20 ms
#define SEARCH_TIME           750

// komunikat chwilowy przez 1.5 sekundy
#define TEMP_MESSAGE_TIME     75

// kontrola: 0.5 s ON / 0.5 s OFF
// 0.5 sekundy = 25 * 20 ms
#define BUZZER_BLINK_TIME     25

// ======================================================
// STANY SYSTEMU
// ======================================================

typedef enum
{
    STATE_IDLE,

    // procedura wejscia
    STATE_ENTRY_UNLOCKED,
    STATE_ENTRY_PASSING,

    // procedura wyjscia
    STATE_EXIT_WAIT_LOTTERY,
    STATE_EXIT_SEARCH,
    STATE_EXIT_UNLOCKED,
    STATE_EXIT_PASSING
} SystemState;

static SystemState state = STATE_IDLE;

static uint8_t peopleCount = 0;

static uint16_t stateTicks = 0;
static uint16_t tempTicks = 0;
static uint16_t rngCounter = 0;

// 0 = brak komunikatu
// 1 = komunikat chwilowy i powrot do tego samego stanu
// 2 = komunikat chwilowy i przejscie do innego stanu
static uint8_t tempMessageMode = 0;
static SystemState tempNextState = STATE_IDLE;

// Poprzednie stany wejsc
static uint8_t lastEntryCard = 0;
static uint8_t lastExitCard = 0;
static uint8_t lastLottery = 0;
static uint8_t lastDoor = 0;

// ======================================================
// LCD HELPERY
// ======================================================

static void LCD_Show2(const char* line1, const char* line2)
{
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_String(line1);

    LCD_SetCursor(1, 0);
    LCD_String(line2);
}

static void LCD_PrintNumber(uint8_t value)
{
    char buffer[4];

    buffer[0] = '0' + (value / 100) % 10;
    buffer[1] = '0' + (value / 10) % 10;
    buffer[2] = '0' + value % 10;
    buffer[3] = '\0';

    if (value < 10)
    {
        LCD_String(&buffer[2]);
    }
    else if (value < 100)
    {
        LCD_String(&buffer[1]);
    }
    else
    {
        LCD_String(buffer);
    }
}

static void LCD_ShowIdle(void)
{
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_String("Idle");

    LCD_SetCursor(1, 0);
    LCD_String("Osoby: ");
    LCD_PrintNumber(peopleCount);
}

// ======================================================
// BUZZER
// ======================================================

static void Buzzer_Init(void)
{
    DDRD |= (1 << BUZZER_PIN);
    PORTD &= ~(1 << BUZZER_PIN);
}

static void Buzzer_On(void)
{
    PORTD |= (1 << BUZZER_PIN);
}

static void Buzzer_Off(void)
{
    PORTD &= ~(1 << BUZZER_PIN);
}

// To dziala tylko dla stanu kontroli.
// W kontroli buzzer piszczy 0.5 s i 0.5 s milczy.
static void Buzzer_Update(void)
{
    if (state == STATE_EXIT_SEARCH)
    {
        if (((stateTicks / BUZZER_BLINK_TIME) % 2) == 0)
        {
            Buzzer_On();
        }
        else
        {
            Buzzer_Off();
        }
    }
}

// ======================================================
// STRZALKI / DIODY
// ======================================================

static void Arrow_Off(void)
{
    HW_ShiftOut(LED_OFF);
}

static void Arrow_Entry(void)
{
    // wejscie: strzalka w prawo
    HW_ShiftOut(STRZALKA_P);
}

static void Arrow_Exit(void)
{
    // wyjscie: strzalka w lewo
    HW_ShiftOut(STRZALKA_L);
}

static void Show_Control_X(void)
{
    // kontrola: wszystkie diody
    HW_ShiftOut(LED_ALL);
}

// ======================================================
// WYSWIETLANIE AKTUALNEGO STANU
// ======================================================

static void RenderState(void)
{
    switch (state)
    {
        case STATE_IDLE:
            Buzzer_Off();
            Arrow_Off();
            LCD_ShowIdle();
            break;

        case STATE_ENTRY_UNLOCKED:
            // Drzwi odblokowane przez 5 sekund.
            // Buzzer piszczy ciagle, dopoki osoba nie nacisnie krancowki.
            Buzzer_On();
            Arrow_Entry();
            LCD_Show2("Prosze wejsc", "Czas: 5 sek");
            break;

        case STATE_ENTRY_PASSING:
            // Osoba zaczela przechodzic.
            // Buzzer i strzalka gasna.
            Buzzer_Off();
            Arrow_Off();
            LCD_Show2("Drzwi otwarte", "Przechodzi...");
            break;

        case STATE_EXIT_WAIT_LOTTERY:
            Buzzer_Off();
            Arrow_Off();
            LCD_Show2("Wcisnij", "losomat");
            break;

        case STATE_EXIT_SEARCH:
            // Kontrola: wszystkie diody + buzzer przerywany.
            // Samym buzzerem steruje Buzzer_Update().
            Buzzer_Off();
            Show_Control_X();
            LCD_Show2("Kontrola!", "15 sekund");
            break;

        case STATE_EXIT_UNLOCKED:
            // Drzwi odblokowane przez 5 sekund.
            // Buzzer piszczy ciagle, dopoki osoba nie nacisnie krancowki.
            Buzzer_On();
            Arrow_Exit();
            LCD_Show2("Prosze wyjsc", "Czas: 5 sek");
            break;

        case STATE_EXIT_PASSING:
            // Osoba zaczela wychodzic.
            // Buzzer i strzalka gasna.
            Buzzer_Off();
            Arrow_Off();
            LCD_Show2("Drzwi otwarte", "Wychodzi...");
            break;

        default:
            Buzzer_Off();
            Arrow_Off();
            LCD_Show2("Blad", "stanu");
            break;
    }
}

static void SetState(SystemState newState)
{
    state = newState;
    stateTicks = 0;
    tempTicks = 0;
    tempMessageMode = 0;

    RenderState();
}

// Komunikat chwilowy, potem powrot do tego samego stanu.
// Timer glownego stanu jest w tym czasie zatrzymany.
// Nie gasimy tutaj buzzera/strzalek, bo procedura dalej trwa.
static void ShowTempReturn(const char* line1, const char* line2)
{
    tempMessageMode = 1;
    tempTicks = 0;

    LCD_Show2(line1, line2);
}

// Komunikat chwilowy, potem przejscie do innego stanu.
// Tu gasimy buzzer i strzalki, bo np. czas minal i drzwi sa blokowane.
static void ShowTempThenGo(const char* line1, const char* line2, SystemState nextState)
{
    tempMessageMode = 2;
    tempTicks = 0;
    tempNextState = nextState;

    Buzzer_Off();
    Arrow_Off();

    LCD_Show2(line1, line2);
}

// ======================================================
// INICJALIZACJA WEJSC
// ======================================================

static void Inputs_Init(void)
{
    // D2-D5 jako wejscia
    DDRD &= ~((1 << BTN_ENTRY_CARD) |
              (1 << BTN_EXIT_CARD) |
              (1 << BTN_LOTTERY) |
              (1 << DOOR_SENSOR));

    // Wlaczenie wewnetrznych pull-upow
    PORTD |= (1 << BTN_ENTRY_CARD);
    PORTD |= (1 << BTN_EXIT_CARD);
    PORTD |= (1 << BTN_LOTTERY);
    PORTD |= (1 << DOOR_SENSOR);
}

// ======================================================
// OBSLUGA KLIKNIEC
// ======================================================

static uint8_t IsClicked(uint8_t currentPressed, uint8_t* lastPressed)
{
    uint8_t clicked = 0;

    if (currentPressed && !(*lastPressed))
    {
        clicked = 1;
    }

    *lastPressed = currentPressed;

    return clicked;
}

static uint8_t Lottery_DrawControl(void)
{
    // 0 = brak kontroli
    // 1 = kontrola
    // okolo 50/50, zalezne od momentu klikniecia
    return (rngCounter & 1);
}

// ======================================================
// MASZYNA STANOW
// ======================================================

static void StateMachine_Update(void)
{
    uint8_t entryCardPressed = PRESSED(BTN_ENTRY_CARD);
    uint8_t exitCardPressed = PRESSED(BTN_EXIT_CARD);
    uint8_t lotteryPressed = PRESSED(BTN_LOTTERY);
    uint8_t doorOpen = PRESSED(DOOR_SENSOR);

    // Jesli pokazujemy chwilowy komunikat,
    // blokujemy logike stanow i tylko czekamy.
    if (tempMessageMode != 0)
    {
        lastEntryCard = entryCardPressed;
        lastExitCard = exitCardPressed;
        lastLottery = lotteryPressed;
        lastDoor = doorOpen;

        if (tempTicks >= TEMP_MESSAGE_TIME)
        {
            if (tempMessageMode == 1)
            {
                // Powrot do tej samej akcji bez resetowania stateTicks.
                tempMessageMode = 0;
                RenderState();
            }
            else if (tempMessageMode == 2)
            {
                // Przejscie do wybranego stanu.
                SystemState next = tempNextState;
                tempMessageMode = 0;
                SetState(next);
            }
        }

        return;
    }

    uint8_t entryCardClick = IsClicked(entryCardPressed, &lastEntryCard);
    uint8_t exitCardClick = IsClicked(exitCardPressed, &lastExitCard);
    uint8_t lotteryClick = IsClicked(lotteryPressed, &lastLottery);

    uint8_t doorOpened = 0;
    uint8_t doorClosed = 0;

    if (doorOpen && !lastDoor)
    {
        doorOpened = 1;
    }

    if (!doorOpen && lastDoor)
    {
        doorClosed = 1;
    }

    lastDoor = doorOpen;

    switch (state)
    {
        // ==================================================
        // IDLE
        // ==================================================
        case STATE_IDLE:
            if (entryCardClick)
            {
                // Karta wejscia rozpoczyna procedure wejscia.
                SetState(STATE_ENTRY_UNLOCKED);
            }
            else if (exitCardClick)
            {
                // Karta wyjscia rozpoczyna procedure wyjscia.
                if (peopleCount > 0)
                {
                    SetState(STATE_EXIT_WAIT_LOTTERY);
                }
                else
                {
                    ShowTempReturn("Brak osob", "w srodku");
                }
            }
            else if (lotteryClick)
            {
                ShowTempReturn("Najpierw", "uzyj karty");
            }
            else if (doorOpen)
            {
                ShowTempReturn("Najpierw", "uzyj karty");
            }
            break;

        // ==================================================
        // WEJSCIE
        // ==================================================
        case STATE_ENTRY_UNLOCKED:
            if (doorOpened)
            {
                // Osoba zaczela przechodzic.
                // Buzzer gasnie w STATE_ENTRY_PASSING.
                SetState(STATE_ENTRY_PASSING);
            }
            else if (exitCardClick || lotteryClick || entryCardClick)
            {
                ShowTempReturn("Niepoprawna", "akcja");
            }
            else if (stateTicks >= ENTRY_UNLOCK_TIME)
            {
                // 5 sekund minelo i nikt nie przeszedl.
                ShowTempThenGo("Czas minal", "Zablokowano", STATE_IDLE);
            }
            break;

        case STATE_ENTRY_PASSING:
            if (doorClosed)
            {
                // Osoba przeszla i puscila krancowke.
                peopleCount++;
                SetState(STATE_IDLE);
            }
            else if (entryCardClick || exitCardClick || lotteryClick)
            {
                ShowTempReturn("Niepoprawna", "akcja");
            }
            break;

        // ==================================================
        // WYJSCIE
        // ==================================================
        case STATE_EXIT_WAIT_LOTTERY:
            if (lotteryClick)
            {
                if (Lottery_DrawControl())
                {
                    // Wylosowano kontrole.
                    SetState(STATE_EXIT_SEARCH);
                }
                else
                {
                    // Brak kontroli, mozna wychodzic.
                    SetState(STATE_EXIT_UNLOCKED);
                }
            }
            else if (doorOpen)
            {
                ShowTempReturn("Najpierw", "losomat");
            }
            else if (entryCardClick || exitCardClick)
            {
                ShowTempReturn("Niepoprawna", "akcja");
            }
            break;

        case STATE_EXIT_SEARCH:
            if (doorOpen)
            {
                ShowTempReturn("Trwa", "kontrola");
            }
            else if (entryCardClick || exitCardClick || lotteryClick)
            {
                ShowTempReturn("Niepoprawna", "akcja");
            }
            else if (stateTicks >= SEARCH_TIME)
            {
                // Po 15 sekundach kontroli mozna wyjsc.
                SetState(STATE_EXIT_UNLOCKED);
            }
            break;

        case STATE_EXIT_UNLOCKED:
            if (doorOpened)
            {
                // Osoba zaczela wychodzic.
                // Buzzer gasnie w STATE_EXIT_PASSING.
                SetState(STATE_EXIT_PASSING);
            }
            else if (entryCardClick || exitCardClick || lotteryClick)
            {
                ShowTempReturn("Niepoprawna", "akcja");
            }
            else if (stateTicks >= EXIT_UNLOCK_TIME)
            {
                // 5 sekund minelo i nikt nie wyszedl.
                ShowTempThenGo("Czas minal", "Zablokowano", STATE_IDLE);
            }
            break;

        case STATE_EXIT_PASSING:
            if (doorClosed)
            {
                // Osoba wyszla i puscila krancowke.
                if (peopleCount > 0)
                {
                    peopleCount--;
                }

                SetState(STATE_IDLE);
            }
            else if (entryCardClick || exitCardClick || lotteryClick)
            {
                ShowTempReturn("Niepoprawna", "akcja");
            }
            break;

        default:
            ShowTempThenGo("Blad", "stanu", STATE_IDLE);
            break;
    }
}

// ======================================================
// MAIN
// ======================================================

int main(void)
{
    HW_Init();
    Inputs_Init();
    Buzzer_Init();

    LCD_Init(0x27);      // jak LCD nie dziala, zmien na 0x3F
    LCD_Clear();

    SetState(STATE_IDLE);

    while (1)
    {
        rngCounter++;

        if (tempMessageMode != 0)
        {
            tempTicks++;
        }
        else
        {
            stateTicks++;
        }

        StateMachine_Update();
        Buzzer_Update();

        _delay_ms(LOOP_DELAY_MS);
    }
}