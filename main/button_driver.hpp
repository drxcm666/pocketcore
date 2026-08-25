#pragma once

#include "driver/gpio.h"

#include <optional>
#include <cstdint>
#include <cstddef>
#include <array>

enum class Button
{
    up,
    down,
    left,
    right,
    ok,
    back,
};

enum class ButtonEventType
{
    press,
    release,
    long_press,
};

struct ButtonEvent
{
    Button button{};
    ButtonEventType type{};
};

struct ButtonState
{
    gpio_num_t gpio{};
    Button button{};

    int stable_state{};
    int last_raw_state{};

    int64_t last_debounce_time{0};
    int64_t last_press_time{0};

    bool is_long_press_handled{false};
    bool valid_press_started{false};
};

struct ButtonEventQueue
{
    std::array<ButtonEvent, 8> events{};

    std::size_t head{0};
    std::size_t tail{0};
    std::size_t count{0};
};

class ButtonDriver
{
private:
    std::array<ButtonState, 6> buttons_ =
        {{{GPIO_NUM_1, Button::ok},
          {GPIO_NUM_2, Button::back},
          {GPIO_NUM_4, Button::up},
          {GPIO_NUM_7, Button::down},
          {GPIO_NUM_5, Button::left},
          {GPIO_NUM_6, Button::right}}};

    ButtonEventQueue queue_;
    bool push_event(const ButtonEvent &event);
    std::optional<ButtonEvent> update_button(ButtonState &state);

public:
    void init();
    void update();
    std::optional<ButtonEvent> pop_event();
};
