#pragma once

#include "application.hpp"
#include "button_driver.hpp"
#include "display.hpp"

class TestApp_2 : public Application
{
private:
    int value_{0};
    Display &display_;

public:
    TestApp_2(Display &display);
    ~TestApp_2() override;

    void enter() override;
    void handle_event(const ButtonEvent &event) override;
    void render() override;
    void exit() override;
};