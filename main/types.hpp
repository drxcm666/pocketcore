#pragma once

enum class AppID
{
    gpio_tools,
    uart_terminal,
    i2c_scanner,
    setting,
};

enum class EventType
{
    none,
    button_press,
    button_release,
    button_long_press,
};

enum class ErrorCode
{
    none,
    invalid_argument,
    device_not_found,
    timeout,
};