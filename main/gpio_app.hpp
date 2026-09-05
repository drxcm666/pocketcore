#pragma once

#include "application.hpp"
#include "display.hpp"
#include "button_driver.hpp"

#include <array>

constexpr std::size_t gpio_menu_size{3};

enum class GpioMode
{
    input,
    output,
};

enum class GpioLevel
{
    low,
    high,
};

struct Element
{
    gpio_num_t gpio;
    GpioMode gpio_mode{GpioMode::input};
    GpioLevel gpio_level{GpioLevel::low};
};

class GpioApp : public Application
{
private:
    Display &display_;

    std::array<Element, 9> gpio_states_{
        Element{GPIO_NUM_13},
        Element{GPIO_NUM_14},
        Element{GPIO_NUM_15},
        Element{GPIO_NUM_16},
        Element{GPIO_NUM_17},
        Element{GPIO_NUM_18},
        Element{GPIO_NUM_21},
        Element{GPIO_NUM_38},
        Element{GPIO_NUM_47},
    };
    std::size_t gpio_index_{0};

    int selected_index_{0};
    int prev_index_{-1};

    int text_x_ = 15;
    int text_y_ = 100;
    int scale_ = 2;

    bool redraw_mode_{false};
    bool redraw_level_{false};

    void draw_selected_item();
    void draw_default_item();

    std::string value_as_string(int row) const;
    gpio_num_t get_gpio() const;

public:
    GpioApp(Display &display);

    void enter() override;
    void handle_event(const ButtonEvent &event) override;
    void render() override;
    void update() override;
    void exit() override;
};