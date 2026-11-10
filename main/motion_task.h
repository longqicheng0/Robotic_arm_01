#pragma once

#include <cstdint>

#include "esp_err.h"

#include "arm_types.h"

namespace arm
{
    esp_err_t runMotionCycle(JointPositions& currentPositions, const JointPositions& targetPositions, const JointLimitsArray& limits, const ServoCalibrationArray& calibrations, float deltaTimeSeconds, std::uint16_t speedStepsPerSecond);

    esp_err_t startMotionTask();
}
