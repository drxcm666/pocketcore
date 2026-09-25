#pragma once

#include "esp_log.h"

#include "stdio.h"

static const char *TAG{"PocketCore | ScopedFile"};

class ScopedFile
{
private:
    FILE *file_{nullptr};

public:
    ScopedFile(const char *path, const char *mode)
    {
        file_ = fopen(path, mode);
        if (file_ == nullptr)
        {
            ESP_LOGE(TAG, "Failed to open file");
        }
    }

    ScopedFile(const ScopedFile &) = delete;
    ScopedFile &operator=(const ScopedFile &) = delete;

    bool is_open() const { return (file_ != nullptr); }
    FILE *get() { return file_; }

    ~ScopedFile()
    {
        if (this->is_open())
        {
            if (fclose(file_) != 0)
            {
                ESP_LOGE(TAG, "Failed to close file. SD card might have been removed or is full");
            }
        }
    }
};