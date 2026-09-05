#include "gpio_app.hpp"

#include <string_view>
#include <string>
#include <utility>
#include <algorithm>

constexpr std::array<std::string_view, gpio_menu_size> gpio_menu_items = {
    "GPIO:",
    "MODE:",
    "LEVEL:",
};

gpio_num_t GpioApp::get_gpio() const
{
    return gpio_states_[gpio_index_].gpio;
}

void set_mode(gpio_num_t gpio, GpioMode mode)
{
    if (mode == GpioMode::input)
    {
        gpio_set_direction(gpio, GPIO_MODE_INPUT);
        gpio_set_pull_mode(gpio, GPIO_PULLDOWN_ONLY);
    }
    else if (mode == GpioMode::output)
    {
        gpio_set_direction(gpio, GPIO_MODE_OUTPUT);
        gpio_set_pull_mode(gpio, GPIO_FLOATING);
    }
}

void set_level(gpio_num_t gpio, GpioLevel level)
{
    if (level == GpioLevel::high)
    {
        gpio_set_level(gpio, 1);
    }
    else if (level == GpioLevel::low)
    {
        gpio_set_level(gpio, 0);
    }
}

std::string GpioApp::value_as_string(int row) const
{
    auto &current_gpio = gpio_states_[gpio_index_];

    switch (row)
    {
    case 0:
        return std::to_string(current_gpio.gpio);
    case 1:
        return current_gpio.gpio_mode == GpioMode::input ? "Input" : "Output";
    case 2:
        return current_gpio.gpio_level == GpioLevel::high ? "High" : "Low";
    default:
        return "";
    }
}

void GpioApp::draw_selected_item()
{
    std::string str = value_as_string(selected_index_);

    std::string text = std::string("> ") + std::string(gpio_menu_items[selected_index_]) + " " + str;
    int right_x = 239;
    int text_height = 7 * scale_;

    display_.fill_rect(0x0000, text_x_, text_y_ + (selected_index_ * 30),
                       right_x, text_y_ + text_height + (selected_index_ * 30));

    display_.draw_text(0xFFFF, text_x_, text_y_ + (selected_index_ * 30), scale_, text);
}

void GpioApp::draw_default_item()
{
    int right_x = 239;
    int text_height = 7 * scale_;

    std::string mode_str;
    if (redraw_mode_)
    {
        mode_str = value_as_string(1);

        display_.fill_rect(0x0000, text_x_, text_y_ + (1 * 30),
                           right_x, text_y_ + text_height + (1 * 30));

        std::string text = std::string(gpio_menu_items[1]) + std::string(" ") + mode_str;
        display_.draw_text(0xFFFF, text_x_, text_y_ + (1 * 30), scale_, text);
    }

    std::string level_str;
    if (redraw_level_)
    {
        level_str = value_as_string(2);

        display_.fill_rect(0x0000, text_x_, text_y_ + (2 * 30),
                           right_x, text_y_ + text_height + (2 * 30));

        std::string text = std::string(gpio_menu_items[2]) + std::string(" ") + level_str;
        display_.draw_text(0xFFFF, text_x_, text_y_ + (2 * 30), scale_, text);
    }

    if (prev_index_ >= 0)
    {
        std::string str = value_as_string(prev_index_);

        display_.fill_rect(0x0000, text_x_, text_y_ + (prev_index_ * 30),
                           right_x, text_y_ + text_height + (prev_index_ * 30));

        std::string text = std::string(gpio_menu_items[prev_index_]) + std::string(" ") + str;
        display_.draw_text(0xFFFF, text_x_, text_y_ + (prev_index_ * 30), scale_, text);
    }
}

GpioApp::GpioApp(Display &display) : display_{display} {}

void GpioApp::enter()
{
    for (const auto &gs : gpio_states_)
    {
        set_mode(gs.gpio, GpioMode::input);
    }

    display_.fill_screen(0x0000);

    std::string gpio_text = std::string(gpio_menu_items[0]) +
                            " " + value_as_string(0);
    std::string mode_text = std::string(gpio_menu_items[1]) +
                            " " + value_as_string(1);
    std::string level_text = std::string(gpio_menu_items[2]) +
                             " " + value_as_string(2);

    display_.draw_text(0xFFFF, text_x_, text_y_ + (0 * 30), scale_, gpio_text);
    display_.draw_text(0xFFFF, text_x_, text_y_ + (1 * 30), scale_, mode_text);
    display_.draw_text(0xFFFF, text_x_, text_y_ + (2 * 30), scale_, level_text);
}

void GpioApp::handle_event(const ButtonEvent &event)
{
    if (event.type == ButtonEventType::press)
    {
        if (event.button == Button::down)
        {
            prev_index_ = selected_index_;

            if (selected_index_ == (gpio_menu_size - 1))
            {
                selected_index_ = 0;
                return;
            }

            selected_index_++;
        }
        else if (event.button == Button::up)
        {
            prev_index_ = selected_index_;

            if (selected_index_ == 0)
            {
                selected_index_ = (gpio_menu_size - 1);
                return;
            }

            selected_index_--;
        }
        else if (event.button == Button::right)
        {
            if (selected_index_ == 0)
            {
                const auto &old_mode = gpio_states_[gpio_index_].gpio_mode;
                const auto &old_level = gpio_states_[gpio_index_].gpio_level;

                if (gpio_index_ >= gpio_states_.size() - 1)
                {
                    gpio_index_ = 0;
                    redraw_mode_ = !(gpio_states_[gpio_index_].gpio_mode == old_mode);
                    redraw_level_ = !(gpio_states_[gpio_index_].gpio_level == old_level);

                    return;
                }

                gpio_index_++;
                redraw_mode_ = !(gpio_states_[gpio_index_].gpio_mode == old_mode);
                redraw_level_ = !(gpio_states_[gpio_index_].gpio_level == old_level);
            }
            else if (selected_index_ == 1)
            {
                auto &mode_ref = gpio_states_[gpio_index_].gpio_mode;
                mode_ref = (mode_ref == GpioMode::input) ? GpioMode::output : GpioMode::input;

                set_mode(get_gpio(), mode_ref);
                if (mode_ref == GpioMode::output)
                {
                    set_level(gpio_states_[gpio_index_].gpio, GpioLevel::low);
                    gpio_states_[gpio_index_].gpio_level = GpioLevel::low;
                    redraw_level_ = true;
                }
            }
            else if (selected_index_ == 2)
            {
                if (gpio_states_[gpio_index_].gpio_mode == GpioMode::output)
                {
                    auto &level_ref = gpio_states_[gpio_index_].gpio_level;
                    level_ref = (level_ref == GpioLevel::low) ? GpioLevel::high : GpioLevel::low;

                    set_level(get_gpio(), level_ref);
                }
            }
        }
        else if (event.button == Button::left)
        {
            if (selected_index_ == 0)
            {
                const auto &old_mode = gpio_states_[gpio_index_].gpio_mode;
                const auto &old_level = gpio_states_[gpio_index_].gpio_level;

                if (gpio_index_ <= 0)
                {
                    gpio_index_ = gpio_states_.size() - 1;
                    redraw_mode_ = !(gpio_states_[gpio_index_].gpio_mode == old_mode);
                    redraw_level_ = !(gpio_states_[gpio_index_].gpio_level == old_level);

                    return;
                }

                gpio_index_--;
                redraw_mode_ = !(gpio_states_[gpio_index_].gpio_mode == old_mode);
                redraw_level_ = !(gpio_states_[gpio_index_].gpio_level == old_level);
            }
            else if (selected_index_ == 1)
            {
                auto &mode_ref = gpio_states_[gpio_index_].gpio_mode;
                mode_ref = (mode_ref == GpioMode::input) ? GpioMode::output : GpioMode::input;

                set_mode(get_gpio(), mode_ref);
                if (mode_ref == GpioMode::output)
                {
                    set_level(gpio_states_[gpio_index_].gpio, GpioLevel::low);
                    gpio_states_[gpio_index_].gpio_level = GpioLevel::low;
                    redraw_level_ = true;
                }
            }
            else if (selected_index_ == 2)
            {
                if (gpio_states_[gpio_index_].gpio_mode == GpioMode::output)
                {
                    auto &level_ref = gpio_states_[gpio_index_].gpio_level;
                    level_ref = (level_ref == GpioLevel::low) ? GpioLevel::high : GpioLevel::low;

                    set_level(get_gpio(), level_ref);
                }
            }
        }
    }
}

void GpioApp::render()
{

    draw_default_item();
    prev_index_ = -1;

    draw_selected_item();

    redraw_mode_ = false;
    redraw_level_ = false;
}

void GpioApp::update()
{
    if (gpio_states_[gpio_index_].gpio_mode == GpioMode::input)
    {
        auto &level_ref = gpio_states_[gpio_index_].gpio_level;
        int lvl = gpio_get_level(gpio_states_[gpio_index_].gpio);

        bool change{false};
        if (lvl == 1 && (level_ref != GpioLevel::high))
        {
            level_ref = GpioLevel::high;
            change = true;
        }
        else if (lvl == 0 && (level_ref != GpioLevel::low))
        {
            level_ref = GpioLevel::low;
            change = true;
        }

        if (change)
        {
            redraw_level_ = true;
            render();
        }
    }
}

void GpioApp::exit()
{
    for (auto &gs : gpio_states_)
    {
        set_mode(gs.gpio, GpioMode::input);
        gs.gpio_mode = GpioMode::input;

        int lvl = gpio_get_level(gs.gpio);
        gs.gpio_level = (lvl == 1) ? GpioLevel::high : GpioLevel::low;
    }

    display_.fill_screen(0x0000);
}