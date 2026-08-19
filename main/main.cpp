#include "esp_log.h"

static const char *TAG{"PocketCore"};

extern "C" void app_main()
{
    ESP_LOGI(TAG, "PocketCore started");
    ESP_LOGW(TAG, "This is a warning");
    ESP_LOGE(TAG, "This is an error");
}
