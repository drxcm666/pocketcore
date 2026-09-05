#include "application_manager.hpp"

void ApplicationManager::open(Application &app)
{
    active_app_ = &app;

    active_app_->enter();
    active_app_->render();
}

void ApplicationManager::process(const ButtonEvent &event)
{
    if (active_app_)
    {
        active_app_->handle_event(event);
        active_app_->render();
    }
}

void ApplicationManager::update()
{
    if (active_app_)
    {
        active_app_->update();
    }
}

void ApplicationManager::close()
{
    if (active_app_)
    {
        active_app_->exit();
        active_app_ = nullptr;
    }
}