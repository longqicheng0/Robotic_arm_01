#pragma once

#include "arm_types.h" 

namespace arm
{
    float clampJointDegrees(float requestedDegrees, const JointLimits& limits);

    float moveTowardDegrees(float currentDegrees, float targetDegrees, float maxDegreesPerSecond, float deltaTimeSeconds);

    float updateJointDegrees(float currentDegrees, float targetDegrees, const JointLimits& limits, float deltaTimeSeconds);

    JointPositions updateJointPositions(const JointPositions& currentPositions, const JointPositions& targetPositions, const JointLimitsArray& limits, float deltaTimeSeconds);

    ServoTargets mapJointPositionsToServoTargets(const JointPositions& currentPositions, const ServoCalibrationArray& calibrations);
}