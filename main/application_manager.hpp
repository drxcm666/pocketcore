#pragma once

#include "application.hpp"

class ApplicationManager
{
private:
    Application *active_app_{nullptr};

public:
    void open(Application &app);
    void process(const ButtonEvent &event);
    void close();
    bool has_active_app() const { return active_app_ != nullptr; }
};