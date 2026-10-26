#include "esp_log.h"

static const char *TAG = "arm_app";

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Robotic arm Phase 1: app_main started");
    ESP_LOGW(TAG, "Hardware outputs are not enabled");
}
