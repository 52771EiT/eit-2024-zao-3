#ifndef LCD_I2C_H
#define LCD_I2C_H

#include <stdint.h>

void LCD_Init(uint8_t i2c_addr);
void LCD_SetCursor(uint8_t row, uint8_t col);
void LCD_String(const char* str);
void LCD_Clear(void);

#endif