#ifndef UI_STATES_H
#define UI_STATES_H

#include <stdint.h>

typedef enum
{
    STATE_IDLE,
    STATE_ENTER,
    STATE_EXIT,
    STATE_PASSING,
    STATE_LOTTERY,
    STATE_SEARCH,
    STATE_LOCK,
    STATE_ERROR,
    STATE_RESET
} SystemState;

void UI_Init(void);
void UI_SetState(SystemState state);
void UI_SetPeopleCount(uint8_t count);
void UI_Update(void);
SystemState UI_GetState(void);

#endif