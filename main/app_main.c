#include "esp_log.h"

static const char *TAG = "arm_app";

void app_main(void)
{
    ESP_LOGI(TAG, "Robotic arm Phase 1: app_main started");
    ESP_LOGW(TAG, "Hardware outputs are not enabled");
}
