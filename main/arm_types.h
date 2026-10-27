#pragma once

#include <cstdint>
#include <array>
#include <cstddef>

namespace arm
{
    //represents what the robot should act
    enum class JointId : std::uint8_t
    {
        Base,
        Shoulder,
        Elbow,
        Gripper,
        Count
    };

    //represent which physical motor receives the result
    enum class ServoIdx : std::uint8_t
    {
        Base,
        ShoulderA,
        ShoulderB,
        Elbow,
        Gripper,
        Count
    };

    struct JointPositions
    {
        std::array<float, static_cast<std::size_t>(JointId::Count)> jointDegrees{};
    };

    struct JointLimits
    {
        float minDegrees{};
        float maxDegrees{};
        float defaultDegrees{};
        float maxDegreesPerSecond{};
    };
    
    using JointLimitsArray = std::array<JointLimits, static_cast<std::size_t>(JointId::Count)>;

    struct ServoTargets
    {
        std::array<std::uint16_t, static_cast<std::size_t>(ServoIdx::Count)> positionSteps{};
    }

}
