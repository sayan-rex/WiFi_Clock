ESP32-C3 Wi-Fi Configuration with Character Increment and Navigation and if Wrong Retake SSID and Password
====================

This ESP32-C3 project integrates Wi-Fi configuration with character increment functionality and navigation. Users can easily set up Wi-Fi credentials using a simple interface. Additionally, if incorrect credentials are entered, the system allows retaking SSID and password by cleaning NVS data.

## Features
- Navigate through characters using right and left buttons.
- Increment button cycles through lowercase letters, uppercase letters, digits, and special characters.
- Wi-Fi SSID and password configuration with user-friendly character input.
- Automatic cleaning of NVS data on incorrect Wi-Fi credentials.

## Requirements
- ESP32-C3 development board
- Three buttons
- Jumper wires

## Hardware Setup
- BUTTON_RIGHT_PIN: Connect to GPIO_NUM_4 on your ESP32-C3 board.
- BUTTON_LEFT_PIN: Connect to GPIO_NUM_5 on your ESP32-C3 board.
- BUTTON_OK_PIN: Connect to GPIO_NUM_0 on your ESP32-C3 board.

## How to Use

1. Connect the buttons to the specified GPIO pins on your ESP32-C3 board.
2. Press the right button to move the cursor to the right.
3. Press the left button to move the cursor to the left.
4. Press the ok button(short press) to cycle through lowercase letters, uppercase letters, digits, and special characters.
5. Configure Wi-Fi SSID and password:

- Use the ok button short press character increment functionality to set the SSID and password characters.
- Ok button First long press will save the SSID and after that prompt "Enter your Password", Enter the passwor through character incremant and navigation using ok button shot press. And after that Ok button Second long press save the Password and connect the AP. 
[N.B- Long press time - 2 seconds]
6. If incorrect Wi-Fi credentials are entered:

- NVS data will be cleared automatically.
- Retake SSID and password using the character increment and navigation interface.
- The current character array is continuously displayed on the console.
