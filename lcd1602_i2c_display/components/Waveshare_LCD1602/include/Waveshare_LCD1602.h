#ifndef WAVESHARE_LCD1602_H
#define WAVESHARE_LCD1602_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"

/*!
 *   Device I2C Address
 */
#define LCD_ADDRESS     (0x7C >> 1)

/*!
 *   Commands
 */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/*!
 *   Flags for display entry mode
 */
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/*!
 *   Flags for display on/off control
 */
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

/*!
 *   Flags for display/cursor shift
 */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/*!
 *   Flags for function set
 */
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x8DOTS 0x00

typedef struct {
    uint8_t cols;
    uint8_t rows;
    uint8_t showfunction;
    uint8_t showcontrol;
    uint8_t showmode;
    uint8_t initialized;
    uint8_t numlines;
    uint8_t currline;
    uint8_t lcdAddr;
    uint8_t RGBAddr;
    uint8_t backlightval;
} Waveshare_LCD1602_t;

esp_err_t Waveshare_LCD1602_init(Waveshare_LCD1602_t *lcd, uint8_t cols, uint8_t rows);
esp_err_t Waveshare_LCD1602_begin(Waveshare_LCD1602_t *lcd, uint8_t cols, uint8_t lines);
esp_err_t Waveshare_LCD1602_home(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_display(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_command(Waveshare_LCD1602_t *lcd, uint8_t cmd);
esp_err_t Waveshare_LCD1602_send(Waveshare_LCD1602_t *lcd, uint8_t *data, uint8_t len);
esp_err_t Waveshare_LCD1602_setCursor(Waveshare_LCD1602_t *lcd, uint8_t col, uint8_t row);
esp_err_t Waveshare_LCD1602_clear(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_write_char(Waveshare_LCD1602_t *lcd, uint8_t value);
esp_err_t Waveshare_LCD1602_send_string(Waveshare_LCD1602_t *lcd, const char *str);
esp_err_t Waveshare_LCD1602_stopBlink(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_blink(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_noCursor(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_cursor(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_scrollDisplayLeft(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_scrollDisplayRight(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_leftToRight(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_rightToLeft(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_noAutoscroll(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_autoscroll(Waveshare_LCD1602_t *lcd);
esp_err_t Waveshare_LCD1602_customSymbol(Waveshare_LCD1602_t *lcd, uint8_t location, uint8_t charmap[]);

#endif // WAVESHARE_LCD1602_H
