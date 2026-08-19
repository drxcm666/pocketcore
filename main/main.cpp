#include "esp_log.h"
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "types.hpp"
#include "gpio_output.hpp"

static const char *TAG{"PocketCore"};

extern "C" void app_main()
{
    GpioOutput led{GPIO_NUM_4};

    while (true)
    {
        led.high();
        ESP_LOGI(TAG, "LED_ON");

        vTaskDelay(pdMS_TO_TICKS(500));

        led.low();
        ESP_LOGI(TAG, "LED_OFF");

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
