#pragma once

#include <cstdint>

#include "esp_err.h"
#include "arm_types.h"

namespace st3215
{
    esp_err_t initialize();

    esp_err_t ping(std::uint8_t id);

    std::uint16_t degreesToPositionSteps(float degrees, const arm::ServoCalibration& calibration);

    esp_err_t writePosition(std::uint8_t id, std::uint16_t positionSteps, std::uint16_t speedStepsPerSecond);
    
    esp_err_t readPosition(std::uint8_t id, std::uint16_t& positionSteps);
    
    esp_err_t synchronizedWrite(const arm::ServoTargets& targets, const arm::ServoCalibrationArray& calibrations, std::uint16_t speedStepsPerSecond);
} // namespace st3215
