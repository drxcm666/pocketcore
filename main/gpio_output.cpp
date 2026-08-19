#include "gpio_output.hpp"

GpioOutput::GpioOutput(gpio_num_t pin) : pin_{pin}
{
    gpio_reset_pin(pin_);
    gpio_set_direction(pin_, GPIO_MODE_OUTPUT);
}

void GpioOutput::high()
{
    gpio_set_level(pin_, 1);
}

void GpioOutput::low()
{
    gpio_set_level(pin_, 0);
}