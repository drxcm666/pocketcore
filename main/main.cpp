#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include <string_view>
#include <array>

#include "display.hpp"
#include "button_driver.hpp"
#include "application.hpp"
#include "test_app.hpp"
#include "test_app_2.hpp"
#include "application_manager.hpp"

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

constexpr std::size_t menu_size{11};

struct MenuItem
{
    std::string_view name;
};

struct MenuParameters
{
    int text_x = 20;
    int text_y = 20;
    int padding = 3;
    int scale = 2;
};

void move_down(std::size_t &selected_index)
{
    if (selected_index == (menu_size - 1))
    {
        selected_index = 0;
        return;
    }

    selected_index++;
}

void move_up(std::size_t &selected_index)
{
    if (selected_index == 0)
    {
        selected_index = (menu_size - 1);
        return;
    }

    selected_index--;
}

constexpr std::array<MenuItem, menu_size> menu_items = {
    MenuItem{"GPIO Tools"},
    MenuItem{"UART Terminal"},
    MenuItem{"I2C Scanner"},
    MenuItem{"Infrared"},
    MenuItem{"NFC"},
    MenuItem{"Sub-GHz"},
    MenuItem{"BLE Scanner"},
    MenuItem{"Wi-Fi Analyzer"},
    MenuItem{"Files"},
    MenuItem{"System Info"},
    MenuItem{"Settings"},
};

void draw_selected_item(Display &display, const MenuParameters &menu, std::size_t i)
{
    int text_width = menu_items[i].name.size() * 6 * menu.scale;
    int text_height = 7 * menu.scale;

    display.fill_rect(0xFFFF, menu.text_x - menu.padding, menu.text_y - menu.padding + (i * 25),
                      menu.text_x + text_width - 1 + menu.padding,
                      menu.text_y + text_height - 1 + menu.padding + (i * 25));

    display.draw_text(0x0000, menu.text_x, menu.text_y + (i * 25), menu.scale, menu_items[i].name);
}

void draw_default_item(Display &display, const MenuParameters &menu, std::size_t i)
{
    int text_width = menu_items[i].name.size() * 6 * menu.scale;
    int text_height = 7 * menu.scale;

    display.fill_rect(0x0000, menu.text_x - menu.padding, menu.text_y - menu.padding + (i * 25),
                      menu.text_x + text_width - 1 + menu.padding,
                      menu.text_y + text_height - 1 + menu.padding + (i * 25));

    display.draw_text(0xFFFF, menu.text_x, menu.text_y + (i * 25), menu.scale, menu_items[i].name);
}

void draw_menu(Display &display, const MenuParameters &menu, std::size_t selected_index)
{
    display.fill_screen(0x0000);

    for (std::size_t i = 0; i < menu_size; i++)
    {
        if (i == selected_index)
        {
            draw_selected_item(display, menu, i);

            continue;
        }

        draw_default_item(display, menu, i);
    }
}

void draw_startup_screen(Display &display)
{
    display.fill_screen(0x0000);
    display.draw_text(0xFFFF, 50, 150, 2, "POCKETCORE\nv0.1");
}

extern "C" void app_main()
{

    std::size_t selected_index{0};
    std::size_t prev_index{0};

    ButtonDriver buttons;
    buttons.init();

    Display display{
        GPIO_NUM_11,
        GPIO_NUM_12,
        GPIO_NUM_10,
        GPIO_NUM_9,
        GPIO_NUM_8,
        40'000'000};

    esp_err_t init_result = display.init();
    if (init_result != ESP_OK)
    {
        ESP_LOGE("PocketCore", "Display init failed: %s", esp_err_to_name(init_result));
        return;
    }

    ESP_LOGI(TAG, "System started! Initial selected menu: %zu", selected_index);

    draw_startup_screen(display);
    vTaskDelay(pdMS_TO_TICKS(1000));

    MenuParameters menu{};
    draw_menu(display, menu, selected_index);

    TestApp test{display};
    TestApp_2 test_2{display};
    ApplicationManager manager{};

    while (true)
    {
        buttons.update();

        while (true)
        {
            auto event = buttons.pop_event();
            if (!event.has_value())
                break;

            if (manager.has_active_app())
            {
                if (event->button == Button::back &&
                    event->type == ButtonEventType::press)
                {
                    manager.close();
                    draw_menu(display, menu, selected_index);
                }
                else
                {
                    manager.process(event.value());
                }
            }
            else
            {
                if (event->button == Button::down &&
                    event->type == ButtonEventType::press)
                {
                    prev_index = selected_index;
                    move_down(selected_index);
                    ESP_LOGI(TAG, "selected: %zu", selected_index);

                    draw_default_item(display, menu, prev_index);
                    draw_selected_item(display, menu, selected_index);
                }
                else if (event->button == Button::up &&
                         event->type == ButtonEventType::press)
                {
                    prev_index = selected_index;
                    move_up(selected_index);
                    ESP_LOGI(TAG, "selected: %zu", selected_index);

                    draw_default_item(display, menu, prev_index);
                    draw_selected_item(display, menu, selected_index);
                }
                else if (event->button == Button::ok &&
                         event->type == ButtonEventType::press &&
                         selected_index == 0)
                {
                    manager.open(test);
                }
                else if (event->button == Button::ok &&
                         event->type == ButtonEventType::press &&
                         selected_index == 1)
                {
                    manager.open(test_2);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
