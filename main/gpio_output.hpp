#pragma once

#include "driver/gpio.h"

class GpioOutput
{
private:
    gpio_num_t pin_;

public:
    explicit GpioOutput(gpio_num_t pin);

    void high();
    void low();
};