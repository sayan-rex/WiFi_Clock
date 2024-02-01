ESP32 C3 ADC with TEMT6000 Light Sensor
====================

This ESP32 C3 project demonstrates reading data from a TEMT6000 light sensor using the ADC (Analog-to-Digital Converter). It continuously reads the sensor data, calculates lux values, and prints the results to the console.


## Features

- Reading light intensity using the TEMT6000 light sensor.
- ADC conversion of raw sensor data to voltage.
- Lux calculation with a basic calibration factor.
- Continuous monitoring with a 1-second delay between readings.

## Requirements

- ESP32 C3 Development Board
- TEMT6000 light sensor

## Hardware Setup

1. Connect the TEMT6000 light sensor to GPIO 4 (TEMT6000_PIN) on your ESP32 C3 development board.

## How to use

1. After flashing the application, open a serial monitor to view the output.
2. The ESP32 C3 will continuously read the TEMT6000 sensor, print raw ADC values, voltage, and lux to the console, and delay for 1 second between readings.



