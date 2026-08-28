#pragma once

#include "application.hpp"
#include "display.hpp"

class TestApp : public Application
{
private:
    int value_{0};
    Display &display_;

public:
    TestApp(Display &display);
    ~TestApp() override;

    void enter() override;

    void handle_event(const ButtonEvent &event) override;

    void render() override;

    void exit() override;
};