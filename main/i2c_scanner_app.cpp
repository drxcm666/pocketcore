#include "esp_log.h"

#include <string>
#include <format>

#include "i2c_scanner_app.hpp"

static const char *TAG{"PocketCore"};

I2cScannerApp::I2cScannerApp(Display &display) : display_{display} 
{
    address_count_ = 0;
    addresses_ = {};
}

void I2cScannerApp::check_i2c_bus()
{
    esp_err_t err;
    for (int i = 0x08; i <= 0x77; i++)
    {
        err = i2c_master_probe(i2c_bus_handle_, i, 50);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Device was found at the address: 0x%02X", i);
            addresses_[address_count_] = i;
            address_count_++;
        }
        else if (err == ESP_ERR_TIMEOUT)
        {
            ESP_LOGI(TAG, "Bus is busy, or problem with the signals");
        }
        else if (err == ESP_ERR_NOT_FOUND)
        {
            continue;
        }
    }
}

void I2cScannerApp::enter()
{
    i2c_master_bus_config_t i2c_bus_config{};
    i2c_bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    i2c_bus_config.i2c_port = -1;
    i2c_bus_config.scl_io_num = GPIO_NUM_14;
    i2c_bus_config.sda_io_num = GPIO_NUM_13;
    i2c_bus_config.glitch_ignore_cnt = 7;
    i2c_bus_config.flags.enable_internal_pullup = true;

    esp_err_t err = i2c_new_master_bus(&i2c_bus_config, &i2c_bus_handle_);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "I2C bus initialized");
        check_i2c_bus(); 
    }
    else
    {
        ESP_LOGE(TAG, "I2C initialization failed: %s", esp_err_to_name(err));
    }
}

void I2cScannerApp::handle_event(const ButtonEvent &event)
{
}

void I2cScannerApp::render()
{
    display_.fill_screen(0x0000);

    display_.draw_text(0xFFFF, text_x_, text_y_, scale_, "Addresses found:");

    std::string hex_str;
    int address_y = text_y_ + 30;
    for (std::size_t i = 0; i < address_count_; i++)
    {
        hex_str = std::format("{:#x}", addresses_[i]);
        display_.draw_text(0xFFFF, text_x_, address_y + (i * 30), scale_, hex_str);
    }
}

void I2cScannerApp::update()
{
}

void I2cScannerApp::exit()
{
    if (i2c_bus_handle_ != nullptr)
    {
        esp_err_t err = i2c_del_master_bus(i2c_bus_handle_);
        if (err != ESP_OK)
        {
            ESP_LOGE(TAG, "Failed to remove I2C bus");
        }
        else
        {
            i2c_bus_handle_ = nullptr;
        }
    }

    addresses_ = {};
    address_count_ = 0;
}