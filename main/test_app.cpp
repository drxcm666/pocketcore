#include "test_app.hpp"

#include "esp_log.h"

#include <string>

static const char *TAG{"TestApp"};

TestApp::TestApp(Display &display) : display_{display} 
{
    ESP_LOGI(TAG, "TestApp constructed");
}

TestApp::~TestApp()
{
    ESP_LOGI(TAG, "TestApp destroyed");
}

void TestApp::enter()
{
    ESP_LOGI(TAG, "TestApp enter");
    value_ = 0;
}

void TestApp::handle_event(const ButtonEvent &event)
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

void TestApp::render()
{
    display_.fill_screen(0x0000);
    display_.draw_text(0xFFFF, 50, 150, 2, "TEST APP\nVALUE: " + std::to_string(value_));
}

void TestApp::exit()
{
    ESP_LOGI(TAG, "TestApp exit");
    display_.fill_screen(0x0000);
}