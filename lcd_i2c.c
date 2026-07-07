#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include "lcd_i2c.h"

#define LCD_BACKLIGHT 0x08
#define LCD_EN 0x04
#define LCD_RS 0x01

static uint8_t lcd_address;

static void I2C_Start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

static void I2C_Stop(void) {
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

static void I2C_Write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

static void LCD_Write_I2C(uint8_t data) {
    I2C_Start();
    I2C_Write(lcd_address);
    I2C_Write(data | LCD_BACKLIGHT);
    I2C_Stop();
}

static void LCD_Pulse_EN(uint8_t data) {
    LCD_Write_I2C(data | LCD_EN);
    _delay_us(1);
    LCD_Write_I2C(data & ~LCD_EN);
    _delay_us(50);
}

static void LCD_Send_Nibble(uint8_t nibble, uint8_t rs) {
    uint8_t data = (nibble & 0xF0) | rs;
    LCD_Write_I2C(data);
    LCD_Pulse_EN(data);
}

static void LCD_Cmd(uint8_t cmd) {
    LCD_Send_Nibble(cmd & 0xF0, 0);
    LCD_Send_Nibble((cmd << 4) & 0xF0, 0);
}

static void LCD_Char(char ch) {
    LCD_Send_Nibble(ch & 0xF0, LCD_RS);
    LCD_Send_Nibble((ch << 4) & 0xF0, LCD_RS);
}

void LCD_Init(uint8_t i2c_addr) {
    lcd_address = (i2c_addr << 1);

    TWSR = 0x00;
    TWBR = 72;
    TWCR = (1 << TWEN);

    _delay_ms(50);

    LCD_Send_Nibble(0x30, 0);
    _delay_ms(5);
    LCD_Send_Nibble(0x30, 0);
    _delay_us(150);
    LCD_Send_Nibble(0x30, 0);
    LCD_Send_Nibble(0x20, 0);

    LCD_Cmd(0x28);
    LCD_Cmd(0x0C);
    LCD_Cmd(0x06);
    LCD_Cmd(0x01);

    _delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t row_offsets[] = {0x00, 0x40};

    if (row > 1) row = 1;
    if (col > 15) col = 15;

    LCD_Cmd(0x80 | (col + row_offsets[row]));
}

void LCD_String(const char* str) {
    while (*str) {
        LCD_Char(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Cmd(0x01);
    _delay_ms(2);
}