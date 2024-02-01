ESP32-C3 SNTP Clock with Wi-Fi Setup using Buttons
====================

This project implements a clock application for the ESP32-C3 microcontroller, allowing users to set up Wi-Fi credentials using buttons with character increment and navigation. The clock synchronizes time with an NTP server using the SNTP protocol.

## Features
- Wi-Fi Connection Setup using buttons for SSID and password entry.
- Navigation through characters using right and left buttons.
- Increment button to cycle through lowercase letters, uppercase letters, digits, and special characters.
- Time synchronization with an NTP server (in.pool.ntp.org).
- Deep sleep mode for power efficiency.

## Requirements
- ESP32 C3 development board
- Three Buttons
- Jumper Wire

## Hardware Setup

- BUTTON_LEFT_PIN ------> GPIO_NUM_4  // No 4 pin of your ESP32 C3 board
- BUTTON_RIGHT_PIN ------> GPIO_NUM_5  // No 5 pin of your ESP32 C3 board
- BUTTON_OK_PIN ------> GPIO_NUM_0  // No 0 pin of your ESP32 C3 board

## How to Use
1. Connect the buttons to the specified GPIO pins.
2. Flash the program to your ESP32 C3 development board.
3. The default `text[]` array serves as a template for both the SSID and password.
4. Users can navigate through the characters in the `text[]` array using the provided buttons.
5. Press the left and right buttons to move the cursor on the prompted `text[]`.
6. Press the OK button to select characters for SSID and password entry.
7. Long-press the OK button to store SSID and password. The first long press will store the SSID, and the second long press will store the password.
8. The program will fetch the current time from the NTP server, display it on the terminal, and then enter deep sleep.
9. The ESP32 C3 wakes up after a specified duration, updates the time, and goes back to sleep.
