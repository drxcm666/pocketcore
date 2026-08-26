#include "display.hpp"
#include "esp_log.h"
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdlib>

static constexpr std::uint8_t font_uppercase[26][7] = {
    // A
    {0b01110, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},

    // B
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110},

    // C
    {0b01111, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b01111},

    // D
    {0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110},

    // E
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111},

    // F
    {0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b10000},

    // G
    {0b01111, 0b10000, 0b10000, 0b10111, 0b10001, 0b10001, 0b01111},

    // H
    {0b10001, 0b10001, 0b10001, 0b11111, 0b10001, 0b10001, 0b10001},

    // I
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b11111},

    // J
    {0b00111, 0b00010, 0b00010, 0b00010, 0b10010, 0b10010, 0b01100},

    // K
    {0b10001, 0b10010, 0b10100, 0b11000, 0b10100, 0b10010, 0b10001},

    // L
    {0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b10000, 0b11111},

    // M
    {0b10001, 0b11011, 0b10101, 0b10101, 0b10001, 0b10001, 0b10001},

    // N
    {0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001},

    // O
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},

    // P
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000, 0b10000},

    // Q
    {0b01110, 0b10001, 0b10001, 0b10001, 0b10101, 0b10010, 0b01101},

    // R
    {0b11110, 0b10001, 0b10001, 0b11110, 0b10100, 0b10010, 0b10001},

    // S
    {0b01111, 0b10000, 0b10000, 0b01110, 0b00001, 0b00001, 0b11110},

    // T
    {0b11111, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100},

    // U
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110},

    // V
    {0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},

    // W
    {0b10001, 0b10001, 0b10001, 0b10101, 0b10101, 0b11011, 0b10001},

    // X
    {0b10001, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001, 0b10001},

    // Y
    {0b10001, 0b10001, 0b01010, 0b00100, 0b00100, 0b00100, 0b00100},

    // Z
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b11111}};

static constexpr std::uint8_t font_lowercase[26][7] = {
    // a
    {0b00000, 0b00000, 0b01110, 0b00001, 0b01111, 0b10001, 0b01111},

    // b
    {0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b10001, 0b11110},

    // c
    {0b00000, 0b00000, 0b01110, 0b10000, 0b10000, 0b10000, 0b01110},

    // d
    {0b00001, 0b00001, 0b01111, 0b10001, 0b10001, 0b10001, 0b01111},

    // e
    {0b00000, 0b00000, 0b01110, 0b10001, 0b11111, 0b10000, 0b01110},

    // f
    {0b00110, 0b01000, 0b11110, 0b01000, 0b01000, 0b01000, 0b01000},

    // g
    {0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110},

    // h
    {0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b10001, 0b10001},

    // i
    {0b00100, 0b00000, 0b01100, 0b00100, 0b00100, 0b00100, 0b01110},

    // j
    {0b00010, 0b00000, 0b00010, 0b00010, 0b00010, 0b10010, 0b01100},

    // k
    {0b10000, 0b10000, 0b10100, 0b11000, 0b11000, 0b10100, 0b10010},

    // l
    {0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},

    // m
    {0b00000, 0b00000, 0b11010, 0b10101, 0b10101, 0b10101, 0b10101},

    // n
    {0b00000, 0b00000, 0b11110, 0b10001, 0b10001, 0b10001, 0b10001},

    // o
    {0b00000, 0b00000, 0b01110, 0b10001, 0b10001, 0b10001, 0b01110},

    // p
    {0b00000, 0b11110, 0b10001, 0b10001, 0b11110, 0b10000, 0b10000},

    // q
    {0b00000, 0b01111, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001},

    // r
    {0b00000, 0b00000, 0b10110, 0b11001, 0b10000, 0b10000, 0b10000},

    // s
    {0b00000, 0b00000, 0b01111, 0b10000, 0b01110, 0b00001, 0b11110},

    // t
    {0b01000, 0b01000, 0b11100, 0b01000, 0b01000, 0b01000, 0b00110},

    // u
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b10011, 0b01101},

    // v
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10001, 0b01010, 0b00100},

    // w
    {0b00000, 0b00000, 0b10001, 0b10001, 0b10101, 0b10101, 0b01010},

    // x
    {0b00000, 0b00000, 0b10001, 0b01010, 0b00100, 0b01010, 0b10001},

    // y
    {0b00000, 0b10001, 0b10001, 0b10001, 0b01111, 0b00001, 0b01110},

    // z
    {0b00000, 0b00000, 0b11111, 0b00010, 0b00100, 0b01000, 0b11111}};

static constexpr std::uint8_t font_digits[10][7] = {
    // 0
    {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110},

    // 1
    {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110},

    // 2
    {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111},

    // 3
    {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110},

    // 4
    {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010},

    // 5
    {0b11111, 0b10000, 0b10000, 0b11110, 0b00001, 0b00001, 0b11110},

    // 6
    {0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110},

    // 7
    {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b01000, 0b01000},

    // 8
    {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110},

    // 9
    {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110}};

static constexpr std::uint8_t glyph_colon[7] = {
    0b00000,
    0b00100,
    0b00100,
    0b00000,
    0b00100,
    0b00100,
    0b00000};

static constexpr std::uint8_t glyph_minus[7] = {
    0b00000,
    0b00000,
    0b00000,
    0b01110,
    0b00000,
    0b00000,
    0b00000};

static constexpr std::uint8_t glyph_dot[7] = {
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00100};

static const char *TAG{"PocketCore"};

static constexpr int width = 240;
static constexpr int height = 320;

Display::Display(gpio_num_t mosi,
                 gpio_num_t sclk,
                 gpio_num_t cs,
                 gpio_num_t dc,
                 gpio_num_t rst,
                 int clock_hz) : mosi_{mosi},
                                 sclk_{sclk},
                                 cs_{cs},
                                 dc_{dc},
                                 rst_{rst},
                                 clock_hz_{clock_hz}
{
}

esp_err_t Display::init()
{
    spi_bus_config_t bus_config{};  // describe the SPI bus itself
    bus_config.mosi_io_num = mosi_; // master out slave in
    bus_config.miso_io_num = -1;    // master in slave out

    bus_config.sclk_io_num = sclk_; /* SPI clock signal.
                                             It tells the display when to read the next bit from MOSI */

    bus_config.quadhd_io_num = -1;
    bus_config.quadwp_io_num = -1;

    bus_config.max_transfer_sz = height * width * 2; // RGB565 -> 16 bits = 2 bytes = 1 pixel

    /* Initialize SPI controller No. 2 using the bus_config settings,
       automatically select DMA, and return the result of the operation */
    esp_err_t result = spi_bus_initialize(
        SPI2_HOST,
        &bus_config,
        SPI_DMA_CH_AUTO); /* DMA helps transfer large blocks of data between RAM and the
                             SPI peripheral without the CPU having to manually handle each byte */

    if (result == ESP_OK)
    {
        ESP_LOGI(TAG, "SPI bus initialized");
    }
    else
    {
        ESP_LOGE(TAG, "SPI initialization failed: %s", esp_err_to_name(result));
        return result;
    }

    spi_device_interface_config_t device_config{}; // describe a specific device on this bus
    device_config.clock_speed_hz = clock_hz_;
    device_config.mode = 0;
    device_config.spics_io_num = cs_;
    device_config.queue_size = 1;

    result = spi_bus_add_device(
        SPI2_HOST,
        &device_config,
        &handle_);

    if (result == ESP_OK)
    {
        ESP_LOGI(TAG, "Display added to SPI bus");
    }
    else
    {
        ESP_LOGE(TAG, "Failed to add display: %s", esp_err_to_name(result));
        return result;
    }

    // Data/Command
    gpio_reset_pin(dc_);
    gpio_set_direction(dc_, GPIO_MODE_OUTPUT);

    // hardware reset pin
    gpio_reset_pin(rst_);
    gpio_set_direction(rst_, GPIO_MODE_OUTPUT);

    gpio_set_level(rst_, 0); // ST7789 reset
    vTaskDelay(pdMS_TO_TICKS(50));
    gpio_set_level(rst_, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // after a reset, the controller may be in sleep mode.
    send_command(0x11); // 0x11 = Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));
    ESP_LOGI(TAG, "ST7789 sleep mode disabled");

    send_command(0x21); // 0x21 = inversion ON

    send_command(0x3A); // 0x3A = pixel format set

    std::uint8_t pixel_format = 0x55; // 0x55 = 16-bit color RGB565
    send_data(&pixel_format, 1);

    send_command(0x29); // 0x29 = display ON
    vTaskDelay(pdMS_TO_TICKS(20));
    fill_screen(0x0000);

    return ESP_OK;
}

void Display::send_command(std::uint8_t command)
{
    gpio_set_level(dc_, 0); // command

    spi_transaction_t transaction{}; // ESP-IDF structure of a single SPI transmission
    transaction.length = 8;
    transaction.tx_buffer = &command; // when the SPI transfer begins, read the data from {&command} in memory

    spi_device_transmit(handle_, &transaction); // transfer of bits from the data memory region via MOSI
}

void Display::send_data(const std::uint8_t *data, std::size_t length)
{
    gpio_set_level(dc_, 1); // data

    spi_transaction_t transaction{}; // ESP-IDF structure of a single SPI transmission
    transaction.length = length * 8;
    transaction.tx_buffer = data; // when the SPI transfer begins, read the data from {data} in memory

    spi_device_transmit(handle_, &transaction); // transfer of bits from the data memory region via MOSI
}

void Display::set_window(std::uint16_t x1, std::uint16_t y1,
                         std::uint16_t x2, std::uint16_t y2)
{
    std::uint8_t x1_high = x1 >> 8;
    std::uint8_t x1_low = x1 & 0xFF;

    std::uint8_t y1_high = y1 >> 8;
    std::uint8_t y1_low = y1 & 0xFF;

    std::uint8_t x2_high = x2 >> 8;
    std::uint8_t x2_low = x2 & 0xFF;

    std::uint8_t y2_high = y2 >> 8;
    std::uint8_t y2_low = y2 & 0xFF;

    send_command(0x2A); // 0x2A = column address set
    std::uint8_t columns[] = {
        x1_high, x1_low,  // 0x00 0x00 = 0x0000 = 0
        x2_high, x2_low}; // 0x00 0xEF = 0x00EF = 239
    send_data(columns, sizeof(columns));

    send_command(0x2B); // 0x2B = row address set
    std::uint8_t rows[] = {
        y1_high, y1_low,
        y2_high, y2_low}; // 0x01 0x3F = 0x013F = 319
    send_data(rows, sizeof(rows));

    send_command(0x2C); // 0x2C = memory write
}

void Display::draw_pixel(std::uint16_t color, int x, int y)
{
    if (x < 0 || x >= width || y < 0 || y >= height)
        return;

    set_window(x, y, x, y);

    std::uint8_t pixel[2];

    std::uint8_t color_hight = color >> 8;
    std::uint8_t color_low = color & 0xFF;

    pixel[0] = color_hight;
    pixel[1] = color_low;

    send_data(pixel, 2);
}

void Display::draw_line(std::uint16_t color,
                        int x1, int y1, int x2, int y2)
{
    int dx = std::abs(x2 - x1);
    int sx = (x2 > x1 ? 1 : -1);

    int dy = std::abs(y2 - y1);
    int sy = (y2 > y1 ? 1 : -1);

    // line runs more along the X-axis
    if (dx >= dy)
    {
        int progress = 0;

        while (true)
        {
            draw_pixel(color, x1, y1);

            if (x1 == x2 && y1 == y2)
                break;

            x1 += sx;

            progress += dy; // progress replaces (dy)  /dx // shows how far we have already progressed toward the secondary axis

            if (progress * 2 >= dx) // (progress >= dx / 2.0) -> (progress / dx >= 0.5)
            {
                y1 += sy;
                progress -= dx;
            }
        }
    }

    // line runs more along the Y-axis
    else
    {
        int progress = 0;

        while (true)
        {
            draw_pixel(color, x1, y1);

            if (x1 == x2 && y1 == y2)
                break;

            y1 += sy;

            progress += dx;

            if (progress * 2 >= dy)
            {
                x1 += sx;
                progress -= dy;
            }
        }
    }
}

void Display::fill_screen(std::uint16_t color)
{
    set_window(0, 0, width - 1, height - 1);

    static std::uint8_t line[width * 2];

    std::uint8_t high = color >> 8;
    std::uint8_t low = color & 0xFF;

    for (int x = 0; x < width; ++x)
    {
        // 0xF800 (16-bit) = red in RGB565
        line[x * 2] = high;
        line[x * 2 + 1] = low;
    }
    // line = [F8 00 F8 00 F8 00 F8 00 ... F8 00]

    for (int y = 0; y < height; ++y)
    {
        send_data(line, sizeof(line));
    }
}

void Display::fill_rect(std::uint16_t color,
                        std::uint16_t x1, std::uint16_t y1,
                        std::uint16_t x2, std::uint16_t y2)
{
    if (x1 > x2 || y1 > y2 || x2 >= width || y2 >= height)
        return;

    set_window(x1, y1, x2, y2);

    int width_pixels = static_cast<int>(x2 - x1 + 1);
    int height_pixels = static_cast<int>(y2 - y1 + 1);

    /*  a buffer in RAM where temporarily store the colors of a
        single horizontal line of the display before sending them via SPI */
    std::uint8_t line[width * 2];

    std::uint8_t color_high = color >> 8;
    std::uint8_t color_low = color & 0xFF;

    for (int x = 0; x < width_pixels; ++x)
    {
        // 0xF800 (16-bit) = red in RGB565
        line[x * 2] = color_high;
        line[x * 2 + 1] = color_low;
    }
    // line = [F8 00 F8 00 F8 00 F8 00 ... F8 00]

    for (int y = 0; y < height_pixels; ++y)
    {
        send_data(line, width_pixels * 2);
    }
}

void Display::draw_letter(std::uint16_t color, int x, int y,
                          std::uint8_t scale, const std::uint8_t *bitmap)
{

    for (int r = 0; r < 7; r++)
    {
        for (int c = 0; c < 5; c++)
        {
            bool bit = (bitmap[r] >> (4 - c)) & 1;
            if (bit == 1)
            {
                fill_rect(color, x + c * scale, y + r * scale,
                          x + ((c + 1) * scale) - 1, y + ((r + 1) * scale) - 1);
            }
        }
    }
}

static const std::uint8_t *get_glyph(char ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return font_uppercase[ch - 'A'];
    }
    else if (ch >= 'a' && ch <= 'z')
    {
        return font_lowercase[ch - 'a'];
    }
    else if (ch >= '0' && ch <= '9')
    {
        return font_digits[ch - '0'];
    }
    else if (ch == ':')
    {
        return glyph_colon;
    }
    else if (ch == '-')
    {
        return glyph_minus;
    }
    else if (ch == '.')
    {
        return glyph_dot;
    }

    return nullptr;
}

void Display::draw_text(std::uint16_t color, int x, int y,
                        std::uint8_t scale, std::string_view text)
{
    int cursor_x = x;
    int cursor_y = y;

    for (const auto ch : text)
    {
        auto bitmap = get_glyph(ch);

        if (ch == '\n')
        {
            cursor_y += 8 * scale;
            cursor_x = x;
            continue;
        }
        if (ch == ' ' || bitmap == nullptr)
        {
            cursor_x += 6 * scale;
            continue;
        }

        draw_letter(color, cursor_x, cursor_y, scale, bitmap);

        cursor_x += 6 * scale;
    }
}