#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_sntp.h"
#include "esp_sleep.h"
#include "esp_err.h"
#include "driver/gpio.h"

#define BUTTON_LEFT_PIN         GPIO_NUM_4
#define BUTTON_RIGHT_PIN        GPIO_NUM_5
#define BUTTON_OK_PIN           GPIO_NUM_0


#define BUTTON_ACTIVE_LEVEL     GPIO_INTR_POSEDGE
#define BUTTON_DEBOUNCE_TIME_MS 5
#define BUTTON_LONG_PRESS_MS    2000

TaskHandle_t buttonTaskHandle;
volatile bool buttonPressed = false;
volatile bool buttonLongPress = false;

static char display[] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
static int cursor = 0;

char temp_ssid[16] = "";
char temp_password[16] = "";
char password[16] = "";
int temp_ssidLength = 0;
int passwordLength = 0;
int temp_passwordLength = 0;
int longPressCount = 0;
char ssid[16] = "";
int ssidLength = 0;

#define ESP_MAXIMUM_RETRY  5

static const char *TAG = "clock";

static int s_retry_num = 0;

static void obtain_time(void);
static void initialize_sntp(void);

void button_isr_handler(void* arg)
{
    buttonPressed = true;
}

static void display_task(void *arg) {
    while (1) {
        // Print the empty display
        printf("\r%s", display);
        fflush(stdout);

        vTaskDelay(150 / portTICK_PERIOD_MS);
    }
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

                    // Delay for additional debounce time
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
                                longPressCount++;

                                if (longPressCount == 1) {
                                    // First long press, print and store SSID
                                    printf("SSID: %s\n", temp_ssid);
                                    strcpy(ssid, temp_ssid);
                                    // Clear the display array
                                        memset(display, ' ', sizeof(display));
                                        display[sizeof(display) - 1] = '\0';
                                        cursor = 0;
                                    printf("Enter your password: \n");
                                }
                                else if (longPressCount == 2) {

                                    printf("Password: %s\n", password);


                                }
                            } else { // OK button short press
                                if (longPressCount == 0) {
                                	if (display[cursor] == ' ') {
                                	                    display[cursor] = 'a';
                                	                } else {
                                	                    if (display[cursor] == 'z') {
                                	                        display[cursor] = 'A'; // Start Capital letters
                                	                    } else if (display[cursor] == 'Z') {
                                	                        display[cursor] = '0'; // Start Digits
                                	                    } else if (display[cursor] == '9') {
                                	                        display[cursor] = '!'; // Start Special characters
                                	                    } else if (display[cursor] == '!') {
                                	                        display[cursor] = '@';
                                	                    } else if (display[cursor] == '@') {
                                	                        display[cursor] = '#';
                                	                    } else if (display[cursor] == '#') {
                                	                        display[cursor] = '$';
                                	                    } else if (display[cursor] == '$') {
                                	                        display[cursor] = '%';
                                	                    } else if (display[cursor] == '%') {
                                	                        display[cursor] = '&';
                                	                    } else if (display[cursor] == '&') {
                                	                        display[cursor] = '*';
                                	                    } else if (display[cursor] == '*') {
                                	                        display[cursor] = '_';
                                	                    } else if (display[cursor] == '_') {
                                	                        display[cursor] = '~';
                                	                    } else if (display[cursor] == '~') {
                                	                        display[cursor] = '^';
                                	                    }else if (display[cursor] == '^') {
                                	                        display[cursor] = ' ';
                                	                    }
                                	                    else {

                                	                    	display[cursor]++;
                                	                    }
                                	                }
                                	                while (gpio_get_level(BUTTON_OK_PIN) == BUTTON_ACTIVE_LEVEL) {
                                	                    vTaskDelay(1 / portTICK_PERIOD_MS);
                                	                }



                                	      // Store the last incremented character for each element of display in SSID
                                	                for (int i = 0; i < sizeof(display) - 1; i++) {
                                	                    if (display[i] != ' ') {
                                	                        temp_ssid[i] = display[i];
                                	                    }
                                	                }
                                	                temp_ssid[sizeof(display) - 1] = '\0';  // Null-terminate the SSID


                                } else if (longPressCount == 1) {
                                	if (display[cursor] == ' ') {
                                	                    display[cursor] = 'a';
                                	                } else {
                                	                    if (display[cursor] == 'z') {
                                	                        display[cursor] = 'A'; // Start Capital letters
                                	                    } else if (display[cursor] == 'Z') {
                                	                        display[cursor] = '0'; // Start Digits
                                	                    } else if (display[cursor] == '9') {
                                	                        display[cursor] = '!'; // Start Special characters
                                	                    } else if (display[cursor] == '!') {
                                	                        display[cursor] = '@';
                                	                    } else if (display[cursor] == '@') {
                                	                        display[cursor] = '#';
                                	                    } else if (display[cursor] == '#') {
                                	                        display[cursor] = '$';
                                	                    } else if (display[cursor] == '$') {
                                	                        display[cursor] = '%';
                                	                    } else if (display[cursor] == '%') {
                                	                        display[cursor] = '&';
                                	                    } else if (display[cursor] == '&') {
                                	                        display[cursor] = '*';
                                	                    } else if (display[cursor] == '*') {
                                	                        display[cursor] = '_';
                                	                    } else if (display[cursor] == '_') {
                                	                        display[cursor] = '~';
                                	                    } else if (display[cursor] == '~') {
                                	                        display[cursor] = '^';
                                	                    }else if (display[cursor] == '^') {
                                	                        display[cursor] = ' ';
                                	                    }
                                	                    else {

                                	                    	display[cursor]++;
                                	                    }
                                	                }
                                	                while (gpio_get_level(BUTTON_OK_PIN) == BUTTON_ACTIVE_LEVEL) {
                                	                    vTaskDelay(10 / portTICK_PERIOD_MS);
                                	                }

                                   // Store the last incremented character for each element of display in password
                                	                                      for (int i = 0; i < sizeof(display) - 1; i++) {
                                	                                         if (display[i] != ' ') {
                                	                                           temp_password[i] = display[i];
                                	                                                  }
                                	                                               }
                                	                        temp_password[sizeof(display) - 1] = '\0';

                                }

                            }
                        }
                    }  // Right button pressed
                    if (gpio_get_level(BUTTON_RIGHT_PIN) == 0) {
                        if (cursor == strlen(display) - 1) {
                            cursor = 0;
                        } else {
                            cursor++;
                        }
                        // Update the display here to reflect the cursor change
                        // You can clear the display and print the new cursor position
                        printf("\r%s", display);
                    }

                    // Left button pressed
                    if (gpio_get_level(BUTTON_LEFT_PIN) == 0) {
                        if (cursor == 0) {
                            cursor = strlen(display) - 1;
                        } else {
                            cursor--;
                        }
                        // Update the display here to reflect the cursor change
                        // You can clear the display and print the new cursor position
                        printf("\r%s", display);
                    }

            }

            buttonPressed = false;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            ESP_LOGI(TAG, "connect to the AP fail");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
    }
}

void connect_to_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_netif_t *wifi_netif = esp_netif_create_default_wifi_sta();
    assert(wifi_netif != NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((char*)wifi_config.sta.ssid, ssid);
    strcpy((char*)wifi_config.sta.password, password);

    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Save SSID and password in NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_config", NVS_READWRITE, &nvs_handle);
    if (err == ESP_OK) {
        err = nvs_set_str(nvs_handle, "ssid", ssid);
        err = nvs_set_str(nvs_handle, "password", password);
        nvs_commit(nvs_handle);
        nvs_close(nvs_handle);
    } else {
        ESP_LOGE(TAG, "Failed to open NVS");
    }

    ESP_LOGI(TAG, "Waiting for IP address...");
}

void app_main(void)
{
    // Initialize NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Retrieve SSID and password from NVS
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("wifi_config", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        size_t ssid_len = sizeof(ssid);
        size_t password_len = sizeof(password);
        err = nvs_get_str(nvs_handle, "ssid", ssid, &ssid_len);
        err = nvs_get_str(nvs_handle, "password", password, &password_len);
        nvs_close(nvs_handle);
    } else {
        ESP_LOGE(TAG, "Failed to open NVS");
    }

    if (strlen(ssid) == 0 || strlen(password) == 0) {
        gpio_config_t io_conf;
        io_conf.intr_type = BUTTON_ACTIVE_LEVEL;
        io_conf.pin_bit_mask = (1ULL << BUTTON_LEFT_PIN) | (1ULL << BUTTON_RIGHT_PIN) | (1ULL << BUTTON_OK_PIN);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_config(&io_conf);
        gpio_install_isr_service(0);
        gpio_isr_handler_add(BUTTON_LEFT_PIN, button_isr_handler, NULL);
        gpio_isr_handler_add(BUTTON_RIGHT_PIN, button_isr_handler, NULL);
        gpio_isr_handler_add(BUTTON_OK_PIN, button_isr_handler, NULL);

        cursor = 0;
        xTaskCreate(button_task, "button_task", 2048, NULL, 10, &buttonTaskHandle);
        xTaskCreate(display_task, "display_task", 2048, NULL, 5, NULL);
    }

    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2023 - 1900)) {
        obtain_time();
    } else {
        ESP_LOGI(TAG, "Time is already set.");
    }

    // Get the current time
       time(&now);
       localtime_r(&now, &timeinfo);

       timeinfo.tm_hour += 5;  // Add 5 hours for India's time zone (UTC+5:30)
       timeinfo.tm_min += 30;  // Add 30 minutes for India's time zone (UTC+5:30)
       mktime(&timeinfo);

       // Format the time in 12-hour format with AM/PM indicator
       char time_str[12];
       strftime(time_str, sizeof(time_str), "%I:%M:%S %p", &timeinfo);

       // Format the date as "Day, Month Date, Year"
       char date_str[20];
       strftime(date_str, sizeof(date_str), "%a, %d/%m/%y", &timeinfo);
       ESP_LOGI(TAG, "Current time: %s, %s", time_str, date_str);

    const int deep_sleep_sec = 60;
    ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
    esp_deep_sleep(1000000LL * deep_sleep_sec);
}

static void obtain_time(void)
{
    if (strlen(ssid) > 0 || strlen(password) > 0) {
        connect_to_wifi();
    } else {
        while (1) {
            if (longPressCount == 2) {
                strcpy(password, temp_password);
                printf("Attempting to connect to Wi-Fi...\n");
                connect_to_wifi();
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    ESP_LOGI(TAG, "Initializing SNTP");
    initialize_sntp();

    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    ESP_ERROR_CHECK(esp_wifi_disconnect());  // Disconnect from Wi-Fi network
}

static void initialize_sntp(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setservername(0, "in.pool.ntp.org");  // NTP server for India
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}
