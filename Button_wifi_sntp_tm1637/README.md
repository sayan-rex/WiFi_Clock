ESP32-C3 SNTP Clock with TM1637 Display
====================

This ESP32-C3 project allows users to set up a Wi-Fi connection, fetch the current time from an NTP server, and display the time on a TM1637 LED display. The program also supports deep sleep mode for power-saving.

## Features

- Wi-Fi Connection Setup using buttons for SSID and password entry.
- Time synchronization with NTP server (in.pool.ntp.org).
- User interface with buttons for character selection, cursor movement, and actions.
- TM1637 LED display for time visualization.
- Deep sleep mode for power efficiency.

## Requirements

- ESP32 C3 development board
- TM1637 LED display
- Three Buttons for Wi-Fi credential input.
- Jumper Wire

## Libraries/Components

The code uses the tm1637 library for interacting with the tm1637 seven segment display. Ensure that this library is available and properly configured in your ESP-IDF project.

## Hardware Setup

- BUTTON_LEFT_PIN         GPIO_NUM_4  // No 4 pin of your ESP32 C3 board
- BUTTON_RIGHT_PIN        GPIO_NUM_5  // No 5 pin of your ESP32 C3 board
- BUTTON_OK_PIN           GPIO_NUM_6  // No 6 pin of your ESP32 C3 board
- PIN_CLK                 GPIO_NUM_0  // No 0 pin of your ESP32 C3 board    
- PIN_DIO                 GPIO_NUM_1  // No 1 pin of your ESP32 C3 board

## How to Use

1. Connect the TM1637 display and buttons to the specified GPIO pins.
2. Flash the program to your ESP32 C3 development board.
3. The default text[] array serves as a template for both the SSID and password.
4. Users can navigate through the characters in the text[] array using the provided buttons.
5. The selected characters are used to form the SSID and password.
6. Press the left and right buttons to move the cursor on the prompted text[].
7. Press the OK button to select characters for SSID and password entry.
8. Long-press the OK button to store SSID and password. First long press will store the SSID and Second long press will store the password.
9. The program will fetch the current time from the NTP server, display it on the TM1637 display, and then enter deep sleep.
10. The ESP32 C3 wakes up after a specified duration, updates the time, and goes back to sleep.


