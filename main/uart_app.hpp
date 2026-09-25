#pragma once

#include "button_driver.hpp"
#include "uart_driver.hpp"
#include "application.hpp"
#include "display.hpp"

#include <array>
#include <cstdint>
#include <deque>
#include <utility>

enum class ViewMode
{
    text,
    hex,
};

class UartTerminalApp : public Application
{
private:
    Display &display_;
    UartDriver &uart_;

    std::array<std::uint8_t, 64> rx_buffer_{};
    std::array<std::uint8_t, 64> line_buffer_{};
    std::deque<std::pair<std::array<std::uint8_t, 64>, std::size_t>> display_lines_{};
    std::size_t line_idx_{0};
    bool upd_screen{false};

    std::size_t max_text_lines_{13};
    std::size_t max_line_symbols_{18};
    bool previous_was_cr_{false};

    ViewMode view_mode_{ViewMode::text};

    void save_line();

public:
    UartTerminalApp(Display &display, UartDriver &uart);
    void enter() override;
    void handle_event(const ButtonEvent &event) override;
    void render() override;
    void update() override;
    void exit() override;
};