#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define TEMT6000_PIN GPIO_NUM_4
#define DEFAULT_VREF    1100        // Default ADC reference voltage in mV (approximate)

static const adc_channel_t TEMT6000_ADC_CHANNEL = ADC_CHANNEL_4; // GPIO 4 is ADC_CHANNEL_4
static const adc_atten_t TEMT6000_ADC_ATTEN = ADC_ATTEN_DB_11;

static esp_adc_cal_characteristics_t *adc_chars;

void initialize_adc()
{
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12); // Use 12-bit ADC resolution
    adc1_config_channel_atten(TEMT6000_ADC_CHANNEL, TEMT6000_ADC_ATTEN);

    // Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, TEMT6000_ADC_ATTEN, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("eFuse Vref\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Two Point\n");
    } else {
        printf("Default\n");
    }
}

void app_main(void)
{
    initialize_adc();

    while (1) {
        // Read ADC value from TEMT6000 sensor
        uint32_t adc_reading = 0;
        for (int i = 0; i < 64; i++) {
            adc_reading += adc1_get_raw(TEMT6000_ADC_CHANNEL);
        }
        adc_reading /= 64;

        // Convert ADC raw value to voltage (in mV)
        uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

        // Calculate Lux using your calibration factor
        float lux = voltage * 1;

        // Print the results
        printf("Raw: %lu\tVoltage: %umV\tLux: %.2f\n", (long unsigned int )adc_reading, (int)voltage, lux);

        // Delay for 1 second before taking the next reading
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
