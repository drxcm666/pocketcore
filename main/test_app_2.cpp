#include "test_app_2.hpp"

#include "esp_log.h"

#include <string>

static const char *TAG{"TestApp_2"};

TestApp_2::TestApp_2(Display &display) : display_{display} 
{
    ESP_LOGI(TAG, "TestApp_2 constructed");
}

TestApp_2::~TestApp_2()
{
    ESP_LOGI(TAG, "TestApp_2 destroyed");
}

void TestApp_2::enter()
{
    ESP_LOGI(TAG, "TestApp_2 enter");
    value_ = 0;
}

void TestApp_2::handle_event(const ButtonEvent &event)
{
    if (event.type == ButtonEventType::press)
    {
        if (event.button == Button::right)
        {
            value_++;
        }
        else if (event.button == Button::left)
        {
            value_--;
        }
        else if (event.button == Button::ok)
        {
            value_ = 0;
        }
    }
}

void TestApp_2::render()
{
    display_.fill_screen(0x0000);
    display_.draw_text(0xFFFF, 50, 150, 2, "TEST APP\nVALUE: " + std::to_string(value_));
}

void TestApp_2::exit()
{
    ESP_LOGI(TAG, "TestApp_2 exit");
    display_.fill_screen(0x0000);
}