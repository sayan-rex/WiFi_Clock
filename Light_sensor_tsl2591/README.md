ESP32-C3 TSL2591 Sensor Exploration
====================

This ESP32-C3 project explores the features of the TSL2591 light sensor. The code initializes the sensor, reads various parameters, and prints the results to the console.

## Features
- Initialization of the TSL2591 light sensor.
- Reading ALS data channels, calculating Lux, and checking various flags.
- Continuous exploration with periodic updates.

## Requirements
- ESP32 C3 development board.
- TSL2591 light sensor.

## Libraries/Components
TSL2591 Library: Ensure that this library is available and properly configured in your ESP-IDF project.

## Hardware Setup

- SDA_PIN -----> No 4 pin of your esp32 c3 board
- SCL_PIN -----> No 5 pin of your esp32 c3 board

## How to Use
- Connect the TSL2591 sensor to the specified GPIO pins on your ESP32 C3 board.
- Flash the program to your ESP32 C3 development board using the ESP-IDF environment.
- Monitor the console output to observe the ALS data, Lux, and various flags.