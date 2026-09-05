#pragma once

#include "button_driver.hpp"

class Application
{
public:
    virtual ~Application() = default;

    virtual void enter() = 0;
    virtual void handle_event(const ButtonEvent &event) = 0;
    virtual void render() = 0;
    virtual void update() = 0;
    virtual void exit() = 0;
};

enum class State
{
    idle,
    running,
    paused
};