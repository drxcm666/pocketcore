#include "uart_driver.hpp"

#include "esp_log.h"
#include "driver/gpio.h"

#include <algorithm>

static const char *TAG{"PocketCore | UartDriver"};

UartDriver::UartDriver(int baud_rate, uart_port_t uart_num,
                       gpio_num_t tx_gpio, gpio_num_t rx_gpio,
                       uart_word_length_t data_bits, uart_parity_t parity,
                       uart_stop_bits_t stop_bits)
    : baud_rate_{baud_rate}, uart_num_{uart_num},
      tx_gpio_{tx_gpio}, rx_gpio_{rx_gpio},
      data_bits_{data_bits}, parity_{parity}, stop_bits_{stop_bits} {}

esp_err_t UartDriver::init()
{
    esp_err_t uart_cfg_res = uart_param_config(uart_num_, &uart_cfg_);
    if (uart_cfg_res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure UART params: %s", esp_err_to_name(uart_cfg_res));
        return uart_cfg_res;
    }

    esp_err_t uart_setp_res = uart_set_pin(uart_num_, tx_gpio_, rx_gpio_,
                                           UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (uart_setp_res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set UART pins: %s", esp_err_to_name(uart_setp_res));
        return uart_setp_res;
    }

    esp_err_t uart_drv_res = uart_driver_install(uart_num_, 256, 0, 0, nullptr, 0);
    if (uart_drv_res != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to install UART driver: %s", esp_err_to_name(uart_drv_res));
        return uart_drv_res;
    }

    driver_installed_ = true;

    return ESP_OK;
}

int UartDriver::read(std::span<std::uint8_t> output)
{
    std::size_t buffered_len{0};
    auto err = uart_get_buffered_data_len(uart_num_, &buffered_len);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "UART failed to get the length of cached RX ring buffer data");
        return -1;
    }

    if (buffered_len > 0)
    {
        return uart_read_bytes(uart_num_, output.data(), std::min(buffered_len, output.size()), 0);
    }

    return 0;
}

int UartDriver::write(std::span<const std::uint8_t> data)
{
    return uart_write_bytes(uart_num_, data.data(), data.size());
}

UartDriver::~UartDriver()
{
    if (driver_installed_)
    {
        uart_driver_delete(uart_num_);
    }
}