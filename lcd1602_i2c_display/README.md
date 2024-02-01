ESP32-C3 Clock with WiFi Configuration
====================

This project implements a clock application for the ESP32-C3 microcontroller. It includes functionality for configuring WiFi credentials using physical buttons and a character-based LCD interface.

## Features:

- Use physical buttons to enter WiFi SSID and password.
- Save the entered WiFi credentials to non-volatile storage (NVS).
- Utilizes a Waveshare LCD1602 display for user interaction.
- Displays a welcome message, prompts for WiFi configuration, and shows the current time.
- Three buttons for navigation and input: Left, Right, and OK.
- OK button: Short press for Select the character from the prompted character string on the terminal, long press for Save the ssid and password.

## Requirements

- ESP32-C3 Development Board
- Waveshare LCD1602 Display
- Three physical buttons for navigation and input (Left, Right, OK)
- Jumper wires

## External Components/Libraries

- Waveshare_LCD1602 library

## Hardware Setup

- BUTTON_LEFT_PIN ---> GPIO_NUM_4 //No 4 pin
- BUTTON_RIGHT_PIN ---> GPIO_NUM_5 // No 5 pin
- BUTTON_OK_PIN ---> GPIO_NUM_6  //No 6 pin
- LCD1602 SDA PIN ---> No 2 pin of your ESP32 C3 board 
- LCD1602 SCL PIN ---> No 3 pin of your ESP32 C3 board 

## How to Use

1. Connect the ESP32-C3, LCD1602 display, and buttons as per Hardware Setup.
2. If WiFi credentials are not saved, use the Left/Right buttons to navigate through printed characters on your terminal and the OK button for Select the character.
3. Follow the on-screen prompts to enter SSID and password.
4. First long press for Save the ssid and Second long press for save the password.
5. After configuring WiFi, the clock displays the current time on the LCD.
6. The ESP32-C3 enters deep sleep for a specific time period.
