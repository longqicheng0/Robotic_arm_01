#pragma once

#include "arm_types.h"

namespace arm{
    CartesianPosition forwardKinematics(
        const JointPositions& jointPositions,
        const ArmGeometry& geometry
    );
}