#include "motion_task.h"

namespace arm
{
    //clamps the requested degrees to be within the joint limits
    float clampJointDegrees(float requestedDegrees, const JointLimits& limits){
        if (requestedDegrees < limits.minDegrees){
            return limits.minDegrees;
        }
        else if (requestedDegrees > limits.maxDegrees){
            return limits.maxDegrees;
        }
        else{
            return requestedDegrees;
        }
    }

    //deltaQ_max = maxDegreesPerSecond * deltaTimeSeconds
    float moveTowardDegrees(float currentDegrees, float targetDegrees, float maxDegreesPerSecond, float deltaTimeSeconds){
        if (maxDegreesPerSecond <= 0 || deltaTimeSeconds <= 0){return currentDegrees;}

        float maxStep = maxDegreesPerSecond * deltaTimeSeconds;
        float error = targetDegrees - currentDegrees;

        if (error >  maxStep){return (currentDegrees + maxStep);}
        if (error < -maxStep){return (currentDegrees - maxStep);}
        return targetDegrees;
    }

    //updates the current degrees toward the target degrees, but does not exceed the joint limits or max degrees per second
    float updateJointDegrees(float currentDegrees, float targetDegrees, const JointLimits& limits, float deltaTimeSeconds){
        float safeTargetDegrees = clampJointDegrees(targetDegrees, limits);

        return moveTowardDegrees(currentDegrees,safeTargetDegrees,limits.maxDegreesPerSecond,deltaTimeSeconds);
    }

    
    JointPositions updateJointPositions(const JointPositions& currentPositions, const JointPositions& targetPositions, const JointLimitsArray& limits, float deltaTimeSeconds){
        JointPositions nextPositions{};

        for (std::size_t i = 0; i < nextPositions.jointDegrees.size(); i++){
            nextPositions.jointDegrees[i] = updateJointDegrees(currentPositions.jointDegrees[i], targetPositions.jointDegrees[i], limits[i], deltaTimeSeconds);
        }

        return nextPositions;
    }
}

