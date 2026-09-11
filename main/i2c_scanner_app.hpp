#pragma once

#include "driver/i2c_master.h"

#include "application.hpp"
#include "button_driver.hpp"
#include "display.hpp"

#include <array>
#include <cstdint>

class I2cScannerApp : public Application
{
private:
    i2c_master_bus_handle_t i2c_bus_handle_{nullptr};
    i2c_master_dev_handle_t i2c_dev_handle_{nullptr};
    Display &display_;

    std::array<int, 112> addresses_{};
    std::size_t address_count_{0};

    int text_x_ = 15;
    int text_y_ = 50;
    int scale_ = 2;

    void check_i2c_bus();

public:
    I2cScannerApp(Display &display);
    void enter() override;
    void handle_event(const ButtonEvent &event) override;
    void render() override;
    void update() override;
    void exit() override;
};