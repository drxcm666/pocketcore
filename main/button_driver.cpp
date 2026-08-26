#include "button_driver.hpp"

#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG{"PocketCore"};

void ButtonDriver::init()
{
    for (auto &btn : buttons_)
    {
        gpio_reset_pin(btn.gpio);
        gpio_set_direction(btn.gpio, GPIO_MODE_INPUT);

        int initial_state = gpio_get_level(btn.gpio);
        btn.stable_state = initial_state;
        btn.last_raw_state = initial_state;
    }
}

void ButtonDriver::update()
{
    for (auto &btn : buttons_)
    {
        auto update_result = update_button(btn);
        if (update_result.has_value())
        {
            ButtonEvent event = update_result.value();
            bool success = push_event(event);
            if (!success)
            {
                ESP_LOGI(TAG, "Event not added. Queue full");
            }
        }
    }
}

std::optional<ButtonEvent> ButtonDriver::update_button(ButtonState &state)
{
    static constexpr int64_t debounce_delay_us{30'000};
    static constexpr int64_t long_press_threshold_us{1'000'000};

    int64_t now = esp_timer_get_time();

    int raw_state = gpio_get_level(state.gpio);

    if (raw_state != state.last_raw_state)
    {
        state.last_debounce_time = now;
        state.last_raw_state = raw_state;
    }

    if ((now - state.last_debounce_time) > debounce_delay_us && raw_state != state.stable_state)
    {
        state.stable_state = raw_state;

        if (state.stable_state == 1)
        {
            state.last_press_time = now;
            state.valid_press_started = true;

            return ButtonEvent{.button = state.button, .type = ButtonEventType::press};
        }
        if (state.stable_state == 0)
        {
            state.is_long_press_handled = false;
            state.valid_press_started = false;

            return ButtonEvent{.button = state.button, .type = ButtonEventType::release};
        }
    }

    if (state.is_long_press_handled == false &&
        state.valid_press_started == true &&
        state.stable_state == 1 &&
        (now - state.last_press_time) > long_press_threshold_us)
    {
        state.is_long_press_handled = true;
        return ButtonEvent{.button = state.button, .type = ButtonEventType::long_press};
    }

    return std::nullopt;
}

bool ButtonDriver::push_event(const ButtonEvent &event)
{
    if (queue_.count == queue_.events.size())
        return false;

    queue_.events.at(queue_.tail) = event;
    queue_.tail = (queue_.tail + 1) % queue_.events.size();
    queue_.count++;

    return true;
}

std::optional<ButtonEvent> ButtonDriver::pop_event()
{
    if (queue_.count == 0)
        return std::nullopt;

    ButtonEvent event = queue_.events.at(queue_.head);

    queue_.head = (queue_.head + 1) % queue_.events.size();
    queue_.count--;

    return event;
}