#pragma once

#include "arm_types.h" 
#include "esp_err.h"

namespace arm
{
    float clampJointDegrees(float requestedDegrees, const JointLimits& limits);

    float moveTowardDegrees(float currentDegrees, float targetDegrees, float maxDegreesPerSecond, float deltaTimeSeconds);

    float updateJointDegrees(float currentDegrees, float targetDegrees, const JointLimits& limits, float deltaTimeSeconds);

    JointPositions updateJointPositions(const JointPositions& currentPositions, const JointPositions& targetPositions, const JointLimitsArray& limits, float deltaTimeSeconds);

    ServoTargets mapJointPositionsToServoTargets(const JointPositions& currentPositions, const ServoCalibrationArray& calibrations);

    esp_err_t runMotionCycle(JointPositions& currentPositions, const JointPositions& targetPositions, const JointLimitsArray& limits, const ServoCalibrationArray& calibrations, float deltaTimeSeconds, std::uint16_t speedStepsPerSecond);

    esp_err_t startMotionTask();
}