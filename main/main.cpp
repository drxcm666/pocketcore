#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "display.hpp"
#include "button_driver.hpp"

const char *to_string(Button btn)
{
    switch (btn)
    {
    case Button::up:
        return "UP";
    case Button::down:
        return "DOWN";
    case Button::left:
        return "LEFT";
    case Button::right:
        return "RIGHT";
    case Button::ok:
        return "OK";
    case Button::back:
        return "BACK";
    default:
        return "UNKNOWN_BTN";
    }
}

const char *to_string(ButtonEventType type)
{
    switch (type)
    {
    case ButtonEventType::press:
        return "PRESS";
    case ButtonEventType::release:
        return "RELEASE";
    case ButtonEventType::long_press:
        return "LONG_PRESS";
    default:
        return "UNKNOWN_EVENT";
    }
}

static const char *TAG{"PocketCore"};

extern "C" void app_main()
{
    ButtonDriver buttons;
    buttons.init();

    while (true)
    {
        buttons.update();

        while (true)
        {
            auto event = buttons.pop_event();
            if (!event.has_value())
                break;

            ESP_LOGI(TAG, "button [ %s ] state is: %s", to_string(event->button), to_string(event->type));
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    /*

    // spi_device_handle_t display_handle;
    Display display{
        GPIO_NUM_11,
        GPIO_NUM_12,
        GPIO_NUM_10,
        GPIO_NUM_9,
        GPIO_NUM_8,
        40'000'000};

    const esp_err_t init_result = display.init();
    if (init_result != ESP_OK)
    {
        ESP_LOGE("PocketCore", "Display init failed: %s", esp_err_to_name(init_result));
        return;
    }

    int64_t start = 0;
    int64_t end = 0;

    // fill_screen()
    start = esp_timer_get_time();

    display.fill_screen(0x0000);

    end = esp_timer_get_time();

    ESP_LOGI(
        TAG,
        "fill_screen: %lld us",
        static_cast<long long>(end - start));

    // large fill_rect()
    start = esp_timer_get_time();

    display.fill_rect(
        0x001F,
        20, 20,
        219, 199);

    end = esp_timer_get_time();

    ESP_LOGI(
        TAG,
        "fill_rect: %lld us",
        static_cast<long long>(end - start));

    // draw_line()
    start = esp_timer_get_time();

    display.draw_line(
        0xF800,
        10, 10,
        220, 280);

    end = esp_timer_get_time();

    ESP_LOGI(
        TAG,
        "draw_line: %lld us",
        static_cast<long long>(end - start));

    // draw_text()
    const std::string performance_text = "POCKETCORE FPS: 60";

    start = esp_timer_get_time();

    display.draw_text(
        0xFFFF,
        20, 220,
        2,
        performance_text);

    end = esp_timer_get_time();

    ESP_LOGI(
        TAG,
        "draw_text: %lld us",
        static_cast<long long>(end - start));

    // -------------------------------------------------
    // Final visual test screen
    // -------------------------------------------------

    display.fill_screen(0x0000);

    // Rectangles
    display.fill_rect(
        0xF800,
        10, 10,
        40, 40);

    display.fill_rect(
        0x07E0,
        50, 10,
        80, 40);

    display.fill_rect(
        0x001F,
        90, 10,
        120, 40);

    // -------------------------------------------------
    // Bresenham tests
    // -------------------------------------------------

    // shallow: right + down
    display.draw_line(
        0xFFFF,
        10, 60,
        220, 100);

    // shallow: left + down
    display.draw_line(
        0xFFE0,
        220, 110,
        10, 150);

    // steep: right + down
    display.draw_line(
        0x07FF,
        20, 160,
        70, 280);

    // steep: left + down
    display.draw_line(
        0xF81F,
        210, 160,
        160, 280);

    // right + up
    display.draw_line(
        0xF800,
        20, 280,
        100, 180);

    // left + up
    display.draw_line(
        0x07E0,
        220, 280,
        140, 180);

    // -------------------------------------------------
    // Text test
    // -------------------------------------------------

    display.draw_text(
        0xFFFF,
        10, 290,
        1,
        "POCKETCORE FPS: 60");

    ESP_LOGI(TAG, "Visual display tests completed");

    */
}