#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>
#include <driver/gpio.h>
#include <esp_sleep.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <sys/time.h>
#include <time.h>
#include "nvs_flash.h"

#define BUTTON_LEFT_PIN         GPIO_NUM_5
#define BUTTON_RIGHT_PIN        GPIO_NUM_2
#define BUTTON_OK_PIN           GPIO_NUM_6

#define BUTTON_ACTIVE_LEVEL     GPIO_INTR_POSEDGE
#define BUTTON_DEBOUNCE_TIME_MS 20
#define BUTTON_LONG_PRESS_MS    2000

#define CURSOR_POS_HOUR         0
#define CURSOR_POS_MINUTE       1
#define CURSOR_POS_SECOND       2
#define CURSOR_POS_AMPM         3
#define CURSOR_POS_WEEKDAY      4
#define CURSOR_POS_DATE         5
#define CURSOR_POS_MONTH        6
#define CURSOR_POS_YEAR         7

TaskHandle_t buttonTaskHandle;
volatile bool buttonPressed = false;
volatile bool buttonLongPress = false;
static int cursorPosition = 0;
int timeArray[8] = {12, 0, 0, 0, 0, 1, 1, 2023};  // Starting with Sunday as the initial weekday
volatile bool updateTimeEnabled = false;
volatile bool timeSavedToRTC = false;
void handleShortPress();

#define RTC_DATA_OFFSET 0 // Offset in RTC memory to store data

static const char *TAG = "clock";

void button_isr_handler(void* arg)
{
    buttonPressed = true;
}

void printTime()
{
    printf("\r%02d:%02d:%02d ", timeArray[0], timeArray[1], timeArray[2]);
    if (timeArray[3] == 0) {
        printf("AM");
    } else {
        printf("PM");
    }

    // Display weekdays (Sunday = 0, Monday = 1, ..., Saturday = 6)
    printf(" %s ",
        timeArray[4] == 0 ? "Sunday" :
        timeArray[4] == 1 ? "Monday" :
        timeArray[4] == 2 ? "Tuesday" :
        timeArray[4] == 3 ? "Wednesday" :
        timeArray[4] == 4 ? "Thursday" :
        timeArray[4] == 5 ? "Friday" : "Saturday"
    );

    // Display date/month/year
    printf(" %02d/%02d/%04d ", timeArray[5], timeArray[6], timeArray[7]);

    for (int i = 0; i < 3 * cursorPosition; i++) {
        printf(" ");
    }
    fflush(stdout);
}

void set_rtc_time()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    struct tm timeinfo;
    time_t now;
    time(&now);
    localtime_r(&now, &timeinfo);

    timeinfo.tm_hour = timeArray[0];
    timeinfo.tm_min = timeArray[1];
    timeinfo.tm_sec = timeArray[2];
    timeinfo.tm_wday = (timeArray[4] + 1) % 7;
    timeinfo.tm_mday = timeArray[5];
    timeinfo.tm_mon = timeArray[6] - 1;  // Month in struct tm is 0-based
    timeinfo.tm_year = timeArray[7] - 1900;  // Years since 1900

    // Adjust for AM/PM
    if (timeArray[3] == 1 && timeinfo.tm_hour < 12) {
        timeinfo.tm_hour += 12; // PM hours
    } else if (timeArray[3] == 0 && timeinfo.tm_hour == 12) {
        timeinfo.tm_hour = 0; // 12 AM is 0 in 24-hour format
    }

    struct timeval tv;
    tv.tv_sec = mktime(&timeinfo);
    tv.tv_usec = 0;

    settimeofday(&tv, NULL);
}

void button_task(void* arg)
{
    TickType_t lastKeyPressTime = 0;
    TickType_t currentTime;

    while (1) {
        if (buttonPressed) {
            currentTime = xTaskGetTickCount();
            if ((currentTime - lastKeyPressTime) * portTICK_PERIOD_MS >= BUTTON_DEBOUNCE_TIME_MS) {
                lastKeyPressTime = currentTime;

                vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_TIME_MS));

                if (gpio_get_level(BUTTON_OK_PIN) == BUTTON_ACTIVE_LEVEL) {
                    // OK button pressed
                    buttonLongPress = false;

                    vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_TIME_MS));

                    if (gpio_get_level(BUTTON_OK_PIN) == BUTTON_ACTIVE_LEVEL) {
                        // OK button still pressed
                        vTaskDelay(pdMS_TO_TICKS(BUTTON_LONG_PRESS_MS));

                        if (gpio_get_level(BUTTON_OK_PIN) == BUTTON_ACTIVE_LEVEL) {
                            // OK button long press
                            buttonLongPress = true;
                            set_rtc_time();
                            printf("\nTime saved to RTC.\n");
                            timeSavedToRTC = true;
                            vTaskSuspend(NULL);
                        } else {
                            // OK button short press
                            handleShortPress();
                        }
                    }
                }
            }

            if (gpio_get_level(BUTTON_LEFT_PIN) == 0) {
                buttonPressed = false;
                cursorPosition = (cursorPosition + 1) % 8;  // Update for new cursor positions
                printTime();
            }

            if (gpio_get_level(BUTTON_RIGHT_PIN) == 0) {
                buttonPressed = false;
                cursorPosition = (cursorPosition - 1 + 8) % 8;  // Update for new cursor positions
                printTime();
            }
        }

        buttonPressed = false;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void handleShortPress() {
    switch (cursorPosition) {
        case CURSOR_POS_HOUR:
            timeArray[0] = (timeArray[0] % 12) + 1;
            break;
        case CURSOR_POS_MINUTE:
            timeArray[1] = (timeArray[1] + 1) % 60;
            break;
        case CURSOR_POS_SECOND:
            timeArray[2] = (timeArray[2] + 1) % 60;
            break;
        case CURSOR_POS_AMPM:
            timeArray[3] = 1 - timeArray[3];
            break;
        case CURSOR_POS_WEEKDAY:
            timeArray[4] = (timeArray[4] + 1) % 7;  // Increment weekday (0-6)
            break;
        case CURSOR_POS_DATE:
            timeArray[5] = (timeArray[5] % 31) + 1;  // Increment date (1-31)
            break;
        case CURSOR_POS_MONTH:
            timeArray[6] = (timeArray[6] % 12) + 1;  // Increment month (1-12)
            break;
        case CURSOR_POS_YEAR:
            timeArray[7] = timeArray[7] + 1;  // Increment year
            break;
    }
    printTime();
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    gpio_config_t io_conf;
    io_conf.intr_type = BUTTON_ACTIVE_LEVEL;
    io_conf.pin_bit_mask = (1ULL << BUTTON_LEFT_PIN) | (1ULL << BUTTON_RIGHT_PIN) | (1ULL << BUTTON_OK_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    gpio_config(&io_conf);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_LEFT_PIN, button_isr_handler, (void*)BUTTON_LEFT_PIN);
    gpio_isr_handler_add(BUTTON_RIGHT_PIN, button_isr_handler, (void*)BUTTON_RIGHT_PIN);
    gpio_isr_handler_add(BUTTON_OK_PIN, button_isr_handler, (void*)BUTTON_OK_PIN);
    // Set the cursor to the first position (hour) at the beginning
    cursorPosition = 0;

    // Print the initial time
    printTime();
    xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);

    while (1) {
        if (!timeSavedToRTC) {
            // Print the initial time
            printTime();
        } else {
            // Print the current time
            time_t now;
            struct tm timeinfo;

            time(&now);
            localtime_r(&now, &timeinfo);

            // Format the time in 12-hour format with AM/PM indicator
            char time_str[12];
            strftime(time_str, sizeof(time_str), "%I:%M:%S %p", &timeinfo);
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%A, %d/%m/%y", &timeinfo);  // Use %A to get full weekday name
            ESP_LOGI(TAG, "Current time: %s, %s", date_str, time_str);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
