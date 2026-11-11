#pragma once

#include <cstdint>

#include "arm_types.h"
#include "servo_config.h"

namespace arm_config {
    inline constexpr arm::JointLimitsArray kJointLimits{
        arm::JointLimits{-180.0f, 180.0f, 0.0f, servo_config::kOperatingSpeedDegreesPerSecond}, // Base
        arm::JointLimits{-90.0f, 90.0f, 0.0f, servo_config::kOperatingSpeedDegreesPerSecond}, // Shoulder
        arm::JointLimits{-0.0f, 180.0f, 90.0f, servo_config::kOperatingSpeedDegreesPerSecond}, // Elbow
        arm::JointLimits{0.0f, 90.0f, 90.0f, servo_config::kOperatingSpeedDegreesPerSecond}     // Gripper
    };

    inline constexpr std::uint32_t kMotionPeriodMS = 20; // 50Hz motion update rate
    inline constexpr float kMotionPeriodSeconds = static_cast<float>(kMotionPeriodMS) / 1000.0f;

    inline constexpr arm::ArmGeometry kArmGeometry{
        0.12306f, // baseHeightMeters
        0.03000f, // shoulderToElbowXMeters
        0.23682f, // shoulderToElbowZMeters
        0.28015f  // elbowToToolMeters
    };
}
