#include "esp_log.h"
#include <cstdint>

#include "types.hpp"

static const char *TAG{"PocketCore"};

extern "C" void app_main()
{
    AppID current_app{AppID::gpio_tools};
    EventType event{EventType::none};
    ErrorCode error{ErrorCode::none};

    int battery_percent{85};
    bool storage_ready{true};
    float battery_voltage{4.12f};

    const int max_apps{10};

    ESP_LOGI(TAG, "Battery: %d%%", battery_percent);
    ESP_LOGI(TAG, "Voltage: %.2f V", battery_voltage);
    ESP_LOGI(TAG, "Storage ready: %s", storage_ready ? "yes" : "no");
    ESP_LOGI(TAG, "Maximum apps: %d", max_apps);
}
