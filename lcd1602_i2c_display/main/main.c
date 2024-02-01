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
#include "Waveshare_LCD1602.h"
#include "driver/i2c.h"
#include "esp_err.h"

#define BUTTON_LEFT_PIN         GPIO_NUM_4
#define BUTTON_RIGHT_PIN        GPIO_NUM_5
#define BUTTON_OK_PIN           GPIO_NUM_6

#define BUTTON_ACTIVE_LEVEL     GPIO_INTR_POSEDGE
#define BUTTON_DEBOUNCE_TIME_MS 50
#define BUTTON_LONG_PRESS_MS    2000

TaskHandle_t buttonTaskHandle;
volatile bool buttonPressed = false;
volatile bool buttonLongPress = false;
bool leftButtonLongPress = false;

int cursorPosition = 0;
char text[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmonpqrstuvwxyz1234567890!@#^&*<>";
char selectedCharacter;
char temp_ssid[100] = "";
char temp_password[100]="";
char password[100] = "";
int temp_ssidLength = 0;
int passwordLength = 0;
int temp_passwordLength = 0;
int longPressCount = 0;
char ssid[100] = "";
int ssidLength = 0;

#define ESP_MAXIMUM_RETRY  5

static const char *TAG = "clock";

static int s_retry_num = 0;

static void obtain_time(void);
static void initialize_sntp(void);
bool ssidSaved = false;
Waveshare_LCD1602_t lcd;
void display_welcome_message(Waveshare_LCD1602_t *lcd) {
    Waveshare_LCD1602_clear(lcd);  // Clear the LCD screen
    Waveshare_LCD1602_setCursor(lcd, 0, 0);
    Waveshare_LCD1602_send_string(lcd, "Welcome to");
    Waveshare_LCD1602_setCursor(lcd, 0, 1);
    Waveshare_LCD1602_send_string(lcd, "ESP32-C3 Clock!");
}

void display_saved_ssid(Waveshare_LCD1602_t *lcd, const char *ssid) {
    Waveshare_LCD1602_clear(lcd);  // Clear the LCD screen
    Waveshare_LCD1602_setCursor(lcd, 0, 0);
    Waveshare_LCD1602_send_string(lcd, "Saved SSID:");
    Waveshare_LCD1602_setCursor(lcd, 0, 1);
    Waveshare_LCD1602_send_string(lcd, ssid);
}

void enter_your_ssid(Waveshare_LCD1602_t *lcd){
    // Display prompt to enter SSID
    	Waveshare_LCD1602_clear(lcd);
        Waveshare_LCD1602_setCursor(lcd, 0, 0);
        Waveshare_LCD1602_send_string(lcd, "Enter SSID:");

}

void enter_your_password(Waveshare_LCD1602_t *lcd){
    Waveshare_LCD1602_clear(lcd);  // Clear the LCD screen
    Waveshare_LCD1602_setCursor(lcd, 0, 0);
    Waveshare_LCD1602_send_string(lcd, "Enter password:");
}

void display_selected_character(Waveshare_LCD1602_t *lcd) {
    // Update the LCD to display the selected character
    Waveshare_LCD1602_clear(lcd);
   // Waveshare_LCD1602_setCursor(lcd, 0, 1);

    // Update the LCD to reflect the changes
    if (longPressCount == 0) {
         // Display "Enter SSID" at (0, 0)
         Waveshare_LCD1602_setCursor(lcd, 0, 0);
         Waveshare_LCD1602_send_string(lcd, "Enter SSID:");

         // Display the entered SSID text at (0, 1)
         Waveshare_LCD1602_setCursor(lcd, 0, 1);
         Waveshare_LCD1602_send_string(lcd, temp_ssid);
     } else if (longPressCount == 1) {
    	 Waveshare_LCD1602_setCursor(lcd, 0, 0);
    	    Waveshare_LCD1602_send_string(lcd, "Enter password:");
    	    Waveshare_LCD1602_setCursor(lcd, 0, 1);
            Waveshare_LCD1602_send_string(lcd, temp_password);
                        }
}

void display_connecting_wifi(Waveshare_LCD1602_t *lcd){
	  // Update the LCD to display "Connecting to Wi-Fi..."
	       Waveshare_LCD1602_clear(lcd);
	       Waveshare_LCD1602_setCursor(lcd, 0, 0);
	       Waveshare_LCD1602_send_string(lcd, "Connecting to");
	       Waveshare_LCD1602_setCursor(lcd, 0, 1);
	       Waveshare_LCD1602_send_string(lcd, "Wi-Fi...");
}


// Function to update the date and time display on the LCD
void update_lcd_datetime(Waveshare_LCD1602_t *lcd, const char *date_str, const char *time_str) {
    Waveshare_LCD1602_clear(lcd);  // Clear the LCD screen
    Waveshare_LCD1602_setCursor(lcd, 0, 0);
    Waveshare_LCD1602_send_string(lcd, time_str);
    Waveshare_LCD1602_setCursor(lcd, 0, 1);
    Waveshare_LCD1602_send_string(lcd, date_str);

}

void button_isr_handler(void* arg)
{
    buttonPressed = true;
}

void printCursor()
{
    printf("\r");  // Move cursor to the beginning of the line
    for (int j = 0; j < strlen(text); j++) {
        if (j == cursorPosition) {
            printf("^");  // Print '^' to indicate cursor position
        } else {
            printf(" ");  // Print spaces for non-cursor positions
        }
    }
    fflush(stdout);
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
                                    // Set a flag to indicate SSID is saved
                                    ssidSaved = true;
                                    printf("Enter your password: \n");
                                	display_saved_ssid(&lcd, ssid);
                                    vTaskDelay(pdMS_TO_TICKS(1000));
                                    // Update the LCD to display "Enter password"
                                    enter_your_password(&lcd);
                                }
                                else if (longPressCount == 2) {

                                    printf("Password: %s\n", password);
                                    display_connecting_wifi(&lcd);

                                }
                            } else { // OK button short press
                                if (longPressCount == 0) {
                                    // Select the character at the cursor position
                                    selectedCharacter = text[cursorPosition];
                                    printf("%c\n", selectedCharacter);

                                    // Save the selected character in SSID
                                    if (temp_ssidLength < sizeof(temp_ssid) - 1) {
                                        temp_ssid[temp_ssidLength] = selectedCharacter;
                                        temp_ssid[temp_ssidLength + 1] = '\0';  // Null-terminate the string
                                        temp_ssidLength++;
                                    }
                                } else if (longPressCount == 1) {
                                    // Select the character at the cursor position
                                    selectedCharacter = text[cursorPosition];
                                    printf("%c\n", selectedCharacter);

                                    // Save the selected character in password
                                    if (temp_passwordLength < sizeof(temp_password) - 1) {
                                        temp_password[temp_passwordLength] = selectedCharacter;
                                        temp_password[temp_passwordLength + 1] = '\0';  // Null-terminate the string
                                        temp_passwordLength++;
                                    }
                                }
                                display_selected_character(&lcd);
                            }
                        }
                    }  else if (gpio_get_level(BUTTON_LEFT_PIN) == BUTTON_ACTIVE_LEVEL) {
                        // Left button pressed
                        if (cursorPosition == 0) {
                            cursorPosition = strlen(text) - 1;  // Move cursor to the last character
                        } else {
                            cursorPosition--;
                        }

                        // Check for long-press
                        leftButtonLongPress = false;
                        vTaskDelay(pdMS_TO_TICKS(BUTTON_DEBOUNCE_TIME_MS));

                        if (gpio_get_level(BUTTON_LEFT_PIN) == BUTTON_ACTIVE_LEVEL) {
                            // Left button still pressed
                            vTaskDelay(pdMS_TO_TICKS(BUTTON_LONG_PRESS_MS));

                            if (gpio_get_level(BUTTON_LEFT_PIN) == BUTTON_ACTIVE_LEVEL) {
                                // Left button long press
                                leftButtonLongPress = true;
                                // Check for backspace functionality
                                if (longPressCount == 0 && temp_ssidLength > 0) {
                                    // Remove the last character from temp_ssid
                                    temp_ssid[temp_ssidLength - 1] = '\0';
                                    temp_ssidLength--;
                                } else if (longPressCount == 1 && temp_passwordLength > 0) {
                                    // Remove the last character from temp_password
                                    temp_password[temp_passwordLength - 1] = '\0';
                                    temp_passwordLength--;
                                }
                                display_selected_character(&lcd);
                            }
                        }
                    }  else if (gpio_get_level(BUTTON_RIGHT_PIN) == BUTTON_ACTIVE_LEVEL) {
                    // Right button pressed
                    if (cursorPosition == strlen(text) - 1) {
                        cursorPosition = 0;  // Move cursor to the first character
                    } else {
                        cursorPosition++;
                    }
                }

                if (cursorPosition < 0) {
                    cursorPosition = 0;
                } else if (cursorPosition >= strlen(text)) {
                    cursorPosition = strlen(text) - 1;
                }

                // Update the cursor and text display
                printCursor();
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
    // Initialize LCD

    Waveshare_LCD1602_init(&lcd, 16, 2);
    Waveshare_LCD1602_begin(&lcd, 16, 2);
    display_welcome_message(&lcd);  // Display welcome message on the LCD
    vTaskDelay(pdMS_TO_TICKS(500));
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
    	enter_your_ssid(&lcd);
        gpio_config_t io_conf;
        io_conf.intr_type = BUTTON_ACTIVE_LEVEL;
        io_conf.pin_bit_mask = (1ULL << BUTTON_LEFT_PIN) | (1ULL << BUTTON_RIGHT_PIN) | (1ULL << BUTTON_OK_PIN);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_config(&io_conf);
        gpio_install_isr_service(0); // Replace ESP_INTR_FLAG_DEFAULT with 0
        gpio_isr_handler_add(BUTTON_LEFT_PIN, button_isr_handler, NULL);
        gpio_isr_handler_add(BUTTON_RIGHT_PIN, button_isr_handler, NULL);
        gpio_isr_handler_add(BUTTON_OK_PIN, button_isr_handler, NULL);

        xTaskCreate(button_task, "button_task", 2048, NULL, 10, &buttonTaskHandle);

        // Print the initial text with the cursor at the first position
        printf("%s\n", text);
        // Print the cursor at its initial position
        printCursor();


    }

    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2023 - 1900)) {
       // ESP_LOGI(TAG, "Time is not set yet. Fetching time over NTP.");
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


    // Display the date and time on the LCD
    update_lcd_datetime(&lcd, date_str, time_str);

    ESP_LOGI(TAG, "Current time: %s, %s", time_str, date_str);

    const int deep_sleep_sec = 60;
       ESP_LOGI(TAG, "Entering deep sleep for %d seconds", deep_sleep_sec);
       esp_deep_sleep(1000000LL * deep_sleep_sec);
}

static void obtain_time(void)
{
	if (strlen(ssid) > 0 || strlen(password) > 0) {
		display_saved_ssid(&lcd, ssid);
		  vTaskDelay(pdMS_TO_TICKS(1000));
		  display_connecting_wifi(&lcd);
		connect_to_wifi();
	}
	else {
    while (1) {

        if (longPressCount == 2) {
        	 strcpy(password, temp_password);
        	      // Add a debug statement to confirm reaching this point
        	    printf("Attempting to connect to Wi-Fi...\n");

        	     // Attempt to connect to Wi-Fi
        	         connect_to_wifi();


            break;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
	}

    ESP_LOGI(TAG, "Initializing SNTP");
    initialize_sntp();

    // Wait for time to be set
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }

    // Disconnect from Wi-Fi network
    ESP_ERROR_CHECK(esp_wifi_disconnect());
    esp_err_t disconnect_result = esp_wifi_disconnect();
    if (disconnect_result == ESP_OK) {
        ESP_LOGI(TAG, "ESP WiFi disconnected");
    } else {
        ESP_LOGE(TAG, "Failed to disconnect ESP WiFi");
    }
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
