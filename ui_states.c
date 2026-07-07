#include "ui_states.h"
#include "lcd_i2c.h"

static SystemState currentState = STATE_IDLE;
static SystemState lastState = STATE_RESET;

static uint8_t peopleCount = 0;
static uint8_t lastPeopleCount = 255;

static void UI_ShowTwoLines(const char* line1, const char* line2)
{
    LCD_Clear();

    LCD_SetCursor(0, 0);
    LCD_String(line1);

    LCD_SetCursor(1, 0);
    LCD_String(line2);
}

void UI_Init(void)
{
    currentState = STATE_IDLE;
    lastState = STATE_RESET;

    peopleCount = 0;
    lastPeopleCount = 255;

    UI_Update();
}

void UI_SetState(SystemState state)
{
    currentState = state;
    UI_Update();
}

void UI_SetPeopleCount(uint8_t count)
{
    peopleCount = count;
    UI_Update();
}

SystemState UI_GetState(void)
{
    return currentState;
}

void UI_Update(void)
{
    if (currentState == lastState && peopleCount == lastPeopleCount)
    {
        return;
    }

    lastState = currentState;
    lastPeopleCount = peopleCount;

    switch (currentState)
    {
        case STATE_IDLE:
        {
            char buffer[4];

            LCD_Clear();

            LCD_SetCursor(0, 0);
            LCD_String("Osoby: ");

            buffer[0] = '0' + (peopleCount / 100) % 10;
            buffer[1] = '0' + (peopleCount / 10) % 10;
            buffer[2] = '0' + peopleCount % 10;
            buffer[3] = '\0';

            if (peopleCount < 10)
            {
                LCD_String(&buffer[2]);
            }
            else if (peopleCount < 100)
            {
                LCD_String(&buffer[1]);
            }
            else
            {
                LCD_String(buffer);
            }

            break;
        }

        case STATE_ENTER:
            UI_ShowTwoLines("Prosze wejsc", "");
            break;

        case STATE_EXIT:
            UI_ShowTwoLines("Prosze wyjsc", "");
            break;

        case STATE_PASSING:
            UI_ShowTwoLines("Drzwi otwarte", "Zamknij drzwi");
            break;

        case STATE_LOTTERY:
            UI_ShowTwoLines("Kliknij", "losomat");
            break;

        case STATE_SEARCH:
            UI_ShowTwoLines("Kontrola!", "Prosze czekac");
            break;

        case STATE_LOCK:
            UI_ShowTwoLines("Najpierw uzyj", "przycisku");
            break;

        case STATE_ERROR:
            UI_ShowTwoLines("Niepoprawna", "akcja");
            break;

        case STATE_RESET:
            UI_ShowTwoLines("Licznik", "wyzerowany");
            break;

        default:
            UI_ShowTwoLines("Blad stanu", "");
            break;
    }
}