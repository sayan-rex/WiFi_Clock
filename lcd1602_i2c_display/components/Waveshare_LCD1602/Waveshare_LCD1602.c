#include "Waveshare_LCD1602.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "esp_err.h"

esp_err_t Waveshare_LCD1602_init(Waveshare_LCD1602_t *lcd, uint8_t cols, uint8_t rows) {
    lcd->cols = cols;
    lcd->rows = rows;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 2,         // select SDA GPIO specific to your project
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = 3,         // select SCL GPIO specific to your project
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 20000,  // select frequency specific to your project
        .clk_flags = 0,        // optional; you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
    };

    esp_err_t err = i2c_param_config(I2C_NUM_0, &conf);
    if (err != ESP_OK) {
        return err;
    }

    err = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        return err;
    }

    lcd->showfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    //Waveshare_LCD1602_begin(lcd);

    return ESP_OK;
}

esp_err_t Waveshare_LCD1602_begin(Waveshare_LCD1602_t *lcd, uint8_t cols, uint8_t lines) {
    if (lines > 1) {
        lcd->showfunction |= LCD_2LINE;
    }
    lcd->numlines = lines;
    lcd->currline = 0;

    // Wait for initialization as per datasheet
    vTaskDelay(pdMS_TO_TICKS(50));

    // Send function set command sequence
    Waveshare_LCD1602_command(lcd, LCD_FUNCTIONSET | lcd->showfunction);
    vTaskDelay(pdMS_TO_TICKS(5));  // Wait more than 4.1ms

    // Second try
    Waveshare_LCD1602_command(lcd, LCD_FUNCTIONSET | lcd->showfunction);
    vTaskDelay(pdMS_TO_TICKS(5));

    // Third go
    Waveshare_LCD1602_command(lcd, LCD_FUNCTIONSET | lcd->showfunction);

    // Turn the display on with no cursor or blinking (default)
    lcd->showcontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    Waveshare_LCD1602_display(lcd);

    // Clear the display
    Waveshare_LCD1602_clear(lcd);

    // Initialize to default text direction
    lcd->showmode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // Set the entry mode
    Waveshare_LCD1602_command(lcd, LCD_ENTRYMODESET | lcd->showmode);

    return ESP_OK;  // Return an appropriate value
}


esp_err_t Waveshare_LCD1602_home(Waveshare_LCD1602_t *lcd)
{
    return Waveshare_LCD1602_command(lcd, LCD_RETURNHOME);
}

esp_err_t Waveshare_LCD1602_display(Waveshare_LCD1602_t *lcd)
{
    lcd-> showcontrol |= LCD_DISPLAYON;
    return Waveshare_LCD1602_command(lcd, LCD_DISPLAYCONTROL | lcd-> showcontrol);
}


esp_err_t Waveshare_LCD1602_command(Waveshare_LCD1602_t *lcd, uint8_t cmd)
{
    uint8_t data[3] = {0x80, cmd};
    return Waveshare_LCD1602_send(lcd, data, 2);
}

esp_err_t Waveshare_LCD1602_send(Waveshare_LCD1602_t *lcd, uint8_t *data, uint8_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (LCD_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, data, len, true);
    i2c_master_stop(cmd);

    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, pdMS_TO_TICKS(1000));
    i2c_cmd_link_delete(cmd);

    return ret;
}

esp_err_t Waveshare_LCD1602_setCursor(Waveshare_LCD1602_t *lcd, uint8_t col, uint8_t row)
{
    uint8_t colAddr[] = {0x80, 0xC0, 0x94, 0xD4}; // Addresses for each row
    uint8_t rowStart = (row < lcd-> numlines) ? colAddr[row] : 0x80; // Default to first row

    col = (col < lcd-> cols) ? col : lcd-> cols - 1; // Limit column to LCD width
    uint8_t data[3] = {0x80, (uint8_t)(rowStart + col)};
    return Waveshare_LCD1602_send(lcd, data, 2);
}

esp_err_t Waveshare_LCD1602_clear(Waveshare_LCD1602_t *lcd)
{
    return Waveshare_LCD1602_command(lcd, LCD_CLEARDISPLAY); // Send command to clear display
}


esp_err_t Waveshare_LCD1602_write_char(Waveshare_LCD1602_t *lcd, uint8_t value)
{
    uint8_t data[3] = {0x40, value};
    return Waveshare_LCD1602_send(lcd, data, 2);
}

esp_err_t Waveshare_LCD1602_send_string(Waveshare_LCD1602_t *lcd, const char *str)
{
    // Loop through the string and send each character
    for (size_t i = 0; str[i] != '\0'; i++) {
        Waveshare_LCD1602_write_char(lcd, (uint8_t)str[i]);
    }
    return ESP_OK;
}

esp_err_t Waveshare_LCD1602_stopBlink(Waveshare_LCD1602_t *lcd)
{
    lcd-> showcontrol &= ~LCD_BLINKON;
    return Waveshare_LCD1602_command(lcd, LCD_DISPLAYCONTROL | lcd-> showcontrol);
}

esp_err_t Waveshare_LCD1602_blink(Waveshare_LCD1602_t *lcd)
{
    lcd-> showcontrol |= LCD_BLINKON;
    return Waveshare_LCD1602_command(lcd, LCD_DISPLAYCONTROL | lcd-> showcontrol);
}


esp_err_t Waveshare_LCD1602_noCursor(Waveshare_LCD1602_t *lcd)
{
    lcd-> showcontrol &= ~LCD_CURSORON;
    return Waveshare_LCD1602_command(lcd, LCD_DISPLAYCONTROL | lcd-> showcontrol);
}

esp_err_t Waveshare_LCD1602_cursor(Waveshare_LCD1602_t *lcd)
{
    lcd-> showcontrol |= LCD_CURSORON;
    return Waveshare_LCD1602_command(lcd, LCD_DISPLAYCONTROL | lcd-> showcontrol);
}

esp_err_t Waveshare_LCD1602_scrollDisplayLeft(Waveshare_LCD1602_t *lcd)
{
    return Waveshare_LCD1602_command(lcd, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

esp_err_t Waveshare_LCD1602_scrollDisplayRight(Waveshare_LCD1602_t *lcd)
{
    return Waveshare_LCD1602_command(lcd, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

esp_err_t Waveshare_LCD1602_leftToRight(Waveshare_LCD1602_t *lcd)
{
    lcd-> showmode |= LCD_ENTRYLEFT;
    return Waveshare_LCD1602_command(lcd, LCD_ENTRYMODESET | lcd-> showmode);
}

esp_err_t Waveshare_LCD1602_rightToLeft(Waveshare_LCD1602_t *lcd)
{
    lcd-> showmode &= ~LCD_ENTRYLEFT;
    return Waveshare_LCD1602_command(lcd, LCD_ENTRYMODESET | lcd-> showmode);
}

esp_err_t Waveshare_LCD1602_noAutoscroll(Waveshare_LCD1602_t *lcd)
{
    lcd-> showmode &= ~LCD_ENTRYSHIFTINCREMENT;
    return Waveshare_LCD1602_command(lcd, LCD_ENTRYMODESET | lcd-> showmode);
}

esp_err_t Waveshare_LCD1602_autoscroll(Waveshare_LCD1602_t *lcd)
{
    lcd-> showmode |= LCD_ENTRYSHIFTINCREMENT;
    return Waveshare_LCD1602_command(lcd, LCD_ENTRYMODESET | lcd-> showmode);
}

esp_err_t Waveshare_LCD1602_customSymbol(Waveshare_LCD1602_t *lcd, uint8_t location, uint8_t charmap[])
{
    location &= 0x7;  // Limit the location to 0-7
    esp_err_t err = Waveshare_LCD1602_command(lcd, LCD_SETCGRAMADDR | (location << 3));
    if (err != ESP_OK) {
        return err;
    }

    uint8_t data[9];
    data[0] = 0x40;
    for (int i = 0; i < 8; i++) {
        data[i + 1] = charmap[i];
    }

    return Waveshare_LCD1602_send(lcd, data, 9);
}
