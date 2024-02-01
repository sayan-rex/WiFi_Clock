//#include <stdio.h>
//#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
//#include <esp_system.h>
//#include <tsl2591.h>
//
//#ifndef APP_CPU_NUM
//#define APP_CPU_NUM PRO_CPU_NUM
//#endif
//
//void tsl2591_test(void *pvParameters)
//{
//    tsl2591_t dev = { 0 };
//
//    ESP_ERROR_CHECK(tsl2591_init_desc(&dev, 0, 4, 5));
//    ESP_ERROR_CHECK(tsl2591_init(&dev));
//
//    // Turn TSL2591 on
//    ESP_ERROR_CHECK(tsl2591_set_power_status(&dev, TSL2591_POWER_ON));
//    // Turn ALS on
//    ESP_ERROR_CHECK(tsl2591_set_als_status(&dev, TSL2591_ALS_ON));
//    // Set gain
//    ESP_ERROR_CHECK(tsl2591_set_gain(&dev, TSL2591_GAIN_MEDIUM));
//    // Set integration time = 300ms
//    ESP_ERROR_CHECK(tsl2591_set_integration_time(&dev, TSL2591_INTEGRATION_300MS));
//
//    float lux;
//    esp_err_t res;
//    while (1)
//    {
//        vTaskDelay(pdMS_TO_TICKS(500));
//
//        if ((res = tsl2591_get_lux(&dev, &lux)) != ESP_OK)
//            printf("Could not read lux value: %d\n", res);
//        else
//            printf("Lux: %f\n", lux);
//    }
//}
//
//void app_main()
//{
//    ESP_ERROR_CHECK(i2cdev_init());
//
//    xTaskCreatePinnedToCore(tsl2591_test, "tsl2591_test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
//}

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <tsl2591.h>

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

void tsl2591_explore_features(void *pvParameters)
{
    tsl2591_t dev = { 0 };

    ESP_ERROR_CHECK(tsl2591_init_desc(&dev, 0, 4, 5));
    ESP_ERROR_CHECK(tsl2591_init(&dev));

    // Turn TSL2591 on
    ESP_ERROR_CHECK(tsl2591_set_power_status(&dev, TSL2591_POWER_ON));
    // Turn ALS on
    ESP_ERROR_CHECK(tsl2591_set_als_status(&dev, TSL2591_ALS_ON));
    // Set gain
    ESP_ERROR_CHECK(tsl2591_set_gain(&dev, TSL2591_GAIN_MEDIUM));
    // Set integration time = 300ms
    ESP_ERROR_CHECK(tsl2591_set_integration_time(&dev, TSL2591_INTEGRATION_300MS));

    uint16_t channel0, channel1;
    float lux;
    bool alsValidFlag, alsIntrFlag, npIntrFlag;
    esp_err_t res;

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Read ALS data channels
        if ((res = tsl2591_get_channel_data(&dev, &channel0, &channel1)) != ESP_OK)
            printf("Could not read channel data: %d\n", res);
        else
            printf("Channel 0: %d, Channel 1: %d\n", channel0, channel1);

        // Calculate Lux
        if ((res = tsl2591_calculate_lux(&dev, channel0, channel1, &lux)) != ESP_OK)
            printf("Could not calculate lux: %d\n", res);
        else
            printf("Lux: %f\n", lux);

        // Check ALS Valid Flag
        if ((res = tsl2591_get_als_valid_flag(&dev, &alsValidFlag)) != ESP_OK)
            printf("Could not read ALS Valid Flag: %d\n", res);
        else
            printf("ALS Valid Flag: %s\n", alsValidFlag ? "Valid" : "Not Valid");

        // Check ALS Interrupt Flag
        if ((res = tsl2591_get_als_intr_flag(&dev, &alsIntrFlag)) != ESP_OK)
            printf("Could not read ALS Interrupt Flag: %d\n", res);
        else
            printf("ALS Interrupt Flag: %s\n", alsIntrFlag ? "Set" : "Cleared");

        // Check No-Persist Interrupt Flag
        if ((res = tsl2591_get_np_intr_flag(&dev, &npIntrFlag)) != ESP_OK)
            printf("Could not read No-Persist Interrupt Flag: %d\n", res);
        else
            printf("No-Persist Interrupt Flag: %s\n", npIntrFlag ? "Set" : "Cleared");
    }
}

void app_main()
{
    ESP_ERROR_CHECK(i2cdev_init());

    xTaskCreatePinnedToCore(tsl2591_explore_features, "tsl2591_explore", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
}




