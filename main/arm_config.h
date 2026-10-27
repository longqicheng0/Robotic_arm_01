#pragma once

#include <cstdint>
#include "driver/uart.h"

namespace arm_config {
    inline constexpr uart_port_t kServoUart       = UART_NUM_1;
    inline constexpr int kServoRxGpio     = 18;
    inline constexpr int kServoTxGpio     = 19;
    inline constexpr int kServoBaudRate   = 1000000;

    inline constexpr std::uint8_t kBaseServoId             = 11;
    inline constexpr std::uint8_t kShoulderDriveServoId    = 12;
    inline constexpr std::uint8_t kShoulderDrivenServoId   = 13;
    inline constexpr std::uint8_t kElbowServoId            = 14;
    inline constexpr std::uint8_t kGripperServoId          = 15;
}
