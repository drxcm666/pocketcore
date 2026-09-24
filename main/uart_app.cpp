#include "uart_app.hpp"

#include "esp_log.h"

#include <string>
#include <format>

static const char *TAG{"PocketCore | UartApp"};

UartTerminalApp::UartTerminalApp(Display &display, UartDriver &uart) : display_{display}, uart_{uart} {}

void UartTerminalApp::enter()
{
    upd_screen = true;
}

void UartTerminalApp::handle_event(const ButtonEvent &event)
{
    if ((event.button == Button::left || event.button == Button::right) && event.type == ButtonEventType::press)
    {
        view_mode_ = (view_mode_ == ViewMode::text) ? ViewMode::hex : ViewMode::text;
        upd_screen = true;
    }
}

void UartTerminalApp::render()
{
    if (upd_screen)
    {
        display_.fill_screen(0x0000);
        display_.draw_text(0xFFFF, 20, 20, 2, "UART" + std::to_string(uart_.get_uart_num()));

        std::string data_bits;
        if (uart_.get_data_bits() == UART_DATA_8_BITS)
        {
            data_bits = "8";
        }
        std::string parity;
        if (uart_.get_parity() == UART_PARITY_DISABLE)
        {
            parity = "N";
        }
        std::string stop_bits;
        if (uart_.get_stop_bits() == UART_STOP_BITS_1)
        {
            stop_bits = "1";
        }

        display_.draw_text(0xFFFF, 100, 20, 2, std::to_string(uart_.get_baud_rate()) + " " + data_bits + parity + stop_bits);
        display_.draw_text(0xFFFF, 20, 40, 2, "RX:" + std::to_string(uart_.get_rx_gpio()));
        display_.draw_text(0xFFFF, 100, 40, 2, "TX:" + std::to_string(uart_.get_tx_gpio()));

        display_.draw_line(0xFFFF, 20, 70, 219, 70);

        std::size_t y = 0;
        int text_height = 7 * 2;

        auto draw_text_line = [&](const std::string &text)
        {
            if (y >= max_text_lines_)
            {
                y = 0;
            }

            display_.fill_rect(0x0000, 20, 80 + static_cast<int>(y) * 18, 239, 80 + static_cast<int>(y) * 18 + text_height);

            display_.draw_text(0xFFFF, 20, 80 + static_cast<int>(y) * 18, 2, text);

            y++;
        };

        for (const auto &l : display_lines_)
        {
            if (view_mode_ == ViewMode::text)
            {
                std::string current_line;

                for (std::size_t i = 0; i < l.second; i++)
                {
                    const char symbol = static_cast<char>(l.first[i]);
                    if (symbol == ' ')
                    {
                        continue;
                    }

                    std::string word;
                    while (i < l.second && l.first[i] != ' ')
                    {

                        word += static_cast<char>(l.first[i]);

                        i++;
                    }

                    if (!current_line.empty() && current_line.size() + 1 + word.size() > max_line_symbols_)
                    {
                        draw_text_line(current_line);
                        current_line.clear();
                    }

                    if (!current_line.empty())
                    {
                        current_line += ' ';
                    }

                    current_line += word;
                }

                if (!current_line.empty())
                {
                    draw_text_line(current_line);
                    current_line.clear();
                }
            }
            else if (view_mode_ == ViewMode::hex)
            {
                std::string current_line;

                for (std::size_t i = 0; i < l.second; i++)
                {
                    std::string hex_byte = std::format("{:02X}", l.first[i]);

                    if (current_line.size() + 3 > max_line_symbols_)
                    {
                        draw_text_line(current_line);
                        current_line.clear();
                    }
                    else if (!current_line.empty())
                    {
                        current_line += ' ';
                    }

                    current_line += hex_byte;
                }

                if (!current_line.empty())
                {
                    draw_text_line(current_line);
                    current_line.clear();
                }
            }
        }

        upd_screen = false;
    }
}

void UartTerminalApp::update()
{
    int rx_bytes = uart_.read(std::span<uint8_t>{rx_buffer_.data(), rx_buffer_.size()});

    if (rx_bytes <= 0)
    {
        return;
    }

    ESP_LOGI(TAG, "Received %d bytes via UART1:", rx_bytes);

    for (int i = 0; i < rx_bytes; i++)
    {
        ESP_LOGI(TAG, "  [%d]: '%c' (0x%02X)", i,
                 (rx_buffer_[i] >= 32 && rx_buffer_[i] <= 126) ? rx_buffer_[i] : '.',
                 rx_buffer_[i]);

        if (rx_buffer_[i] == '\r')
        {
            display_lines_.push_back({line_buffer_, line_idx_});
            if (display_lines_.size() > max_text_lines_)
            {
                display_lines_.pop_front();
            }

            line_idx_ = 0;
            upd_screen = true;

            previous_was_cr_ = true;
        }
        else if (rx_buffer_[i] == '\n')
        {
            if (previous_was_cr_)
            {
                previous_was_cr_ = false;
                continue;
            }
            else
            {
                display_lines_.push_back({line_buffer_, line_idx_});
                if (display_lines_.size() > max_text_lines_)
                {
                    display_lines_.pop_front();
                }

                line_idx_ = 0;
                upd_screen = true;
            }

            previous_was_cr_ = false;
        }
        else
        {
            previous_was_cr_ = false;

            if (line_idx_ < line_buffer_.size())
            {
                line_buffer_[line_idx_++] = rx_buffer_[i];
            }
            else
            {
                display_lines_.push_back({line_buffer_, line_idx_});
                if (display_lines_.size() > max_text_lines_)
                {
                    display_lines_.pop_front();
                }

                upd_screen = true;

                line_idx_ = 0;
                line_buffer_[line_idx_++] = rx_buffer_[i];
            }
        }
    }
}

void UartTerminalApp::exit()
{
}