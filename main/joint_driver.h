#pragma once

#include <cstdint>

#include "esp_err.h"
#include "arm_types.h"

namespace st3215
{
    esp_err_t initialize();
    esp_err_t ping(std::uint8_t id);
    std::uint16_t degreesToPositionSteps(float degrees, const arm::ServoCalibration& calibration);
} // namespace st3215
