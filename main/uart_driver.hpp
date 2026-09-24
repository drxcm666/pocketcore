#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"

#include <cstdint>
#include <span>

class UartDriver
{
private:
    int baud_rate_{115200};

    uart_port_t uart_num_;
    gpio_num_t tx_gpio_;
    gpio_num_t rx_gpio_;
    uart_word_length_t data_bits_{UART_DATA_8_BITS};
    uart_parity_t parity_{UART_PARITY_DISABLE};
    uart_stop_bits_t stop_bits_{UART_STOP_BITS_1};

    uart_config_t uart_cfg_{
        .baud_rate = baud_rate_,
        .data_bits = data_bits_,
        .parity = parity_,
        .stop_bits = stop_bits_,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,
        .source_clk = UART_SCLK_DEFAULT,
        .flags = {}};

    bool driver_installed_{false};

public:
    UartDriver(int baud_rate, uart_port_t uart_num,
               gpio_num_t tx_gpio, gpio_num_t rx_gpio,
               uart_word_length_t data_bits, uart_parity_t parity,
               uart_stop_bits_t stop_bits);
    esp_err_t init();
    int read(std::span<std::uint8_t> output);
    int write(std::span<const std::uint8_t> data);
    ~UartDriver();

    uart_port_t get_uart_num() const { return uart_num_; }
    int get_baud_rate() const { return baud_rate_; }
    gpio_num_t get_tx_gpio() const { return tx_gpio_; };
    gpio_num_t get_rx_gpio() const { return rx_gpio_; };
    uart_word_length_t get_data_bits() const { return data_bits_; };
    uart_parity_t get_parity() const { return parity_; };
    uart_stop_bits_t get_stop_bits() const { return stop_bits_; };
};