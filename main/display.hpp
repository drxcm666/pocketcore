#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <cstdint>
#include <cstddef>
#include <string>

class Display
{
private:
    spi_device_handle_t handle_{nullptr};

    gpio_num_t mosi_;
    gpio_num_t sclk_;
    gpio_num_t cs_;
    gpio_num_t dc_;
    gpio_num_t rst_;
    int clock_hz_;

    void send_command(std::uint8_t command);
    void send_data(const std::uint8_t *data, std::size_t length);
    void set_window(std::uint16_t x1, std::uint16_t y1,
                    std::uint16_t x2, std::uint16_t y2);
    void draw_letter(std::uint16_t color, int x, int y, std::uint8_t scale, const std::uint8_t *bitmap);

public:
    Display(gpio_num_t mosi, gpio_num_t sclk, gpio_num_t cs, gpio_num_t dc, gpio_num_t rst, int clock_hz);

    esp_err_t init();
    void draw_pixel(std::uint16_t color, int x, int y);
    void draw_line(std::uint16_t color, int x1, int y1, int x2, int y2);
    void draw_text(std::uint16_t color, int x, int y, std::uint8_t scale, const std::string &text);
    void fill_screen(std::uint16_t color);
    void fill_rect(std::uint16_t color,
                   std::uint16_t x1, std::uint16_t y1,
                   std::uint16_t x2, std::uint16_t y2);
};