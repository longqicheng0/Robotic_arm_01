#include "esp_log.h"
#include "joint_driver.h"

static const char *TAG = "arm_app";

extern "C" void app_main()
{
    ESP_LOGI(TAG, "Robotic arm Phase 1: app_main started");

    esp_err_t init = st3215::initialize();
    if (init != ESP_OK){
        ESP_LOGE(TAG, "UART init failed, Exiting...");
        return;
    }

}
