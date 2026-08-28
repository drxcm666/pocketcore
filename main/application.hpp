#pragma once

#include "button_driver.hpp"

class Application
{
public:
    virtual ~Application() = default;

    virtual void enter() = 0;
    virtual void handle_event(const ButtonEvent &event) = 0;
    virtual void render() = 0;
    virtual void exit() = 0;
};

enum class State
{
    idle,
    running,
    paused
};

// void handle_state(State &state, const Button &button)
// {
//     if (state == State::idle && button == Button::ok)
//         state = State::running;
    
//     else if (state == State::running && button == Button::ok)
//         state = State::paused;
    
//     else if (state == State::paused && button == Button::ok)
//         state = State::running;

//     else if (state == State::running && button == Button::back)
//         state = State::idle;

//     else if (state == State::paused && button == Button::back)
//         state = State::idle;

//     else if (state == State::idle && button == Button::back)
//         state = State::idle;
// }