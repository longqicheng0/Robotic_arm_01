#pragma once

#include <cstdint>
#include "driver/uart.h"
#include "arm_types.h"

namespace arm_config {
    inline constexpr uart_port_t kServoUart       = UART_NUM_1;
    inline constexpr int kServoRxGpio     = 18;
    inline constexpr int kServoTxGpio     = 19;
    inline constexpr int kServoBaudRate   = 1000000;

    inline constexpr std::uint8_t kBaseServoId             = 11;
    inline constexpr std::uint8_t kShoulderAServoId    = 12;
    inline constexpr std::uint8_t kShoulderBServoId   = 13;
    inline constexpr std::uint8_t kElbowServoId            = 14;
    inline constexpr std::uint8_t kGripperServoId          = 15;

    inline constexpr std::uint16_t kMaxSpeedStepsPerSecond = 3073;
    inline constexpr std::uint16_t kOperatingSpeedStepsPerSecond = kMaxSpeedStepsPerSecond * 95 / 100;

    inline constexpr float kOperatingSpeedDegreesPerSecond = 360.0f * kOperatingSpeedStepsPerSecond / 4096.0f;
    inline constexpr arm::JointLimitsArray kJointLimits{
        arm::JointLimits{-180.0f, 180.0f, 0.0f, kOperatingSpeedDegreesPerSecond}, // Base
        arm::JointLimits{-90.0f, 90.0f, 0.0f, kOperatingSpeedDegreesPerSecond}, // Shoulder
        arm::JointLimits{-0.0f, 180.0f, 90.0f, kOperatingSpeedDegreesPerSecond}, // Elbow
        arm::JointLimits{0.0f, 90.0f, 90.0f, kOperatingSpeedDegreesPerSecond}     // Gripper
    };

    inline constexpr arm::ServoCalibrationArray kServoCalibrations{
        arm::ServoCalibration{kBaseServoId, 2047, true}, // Base
        arm::ServoCalibration{kShoulderAServoId, 2047, false}, // ShoulderA
        arm::ServoCalibration{kShoulderBServoId, 2047, true}, // ShoulderB
        arm::ServoCalibration{kElbowServoId, 1024, false}, // Elbow
        arm::ServoCalibration{kGripperServoId, 2047, true} // Gripper
    };
}
