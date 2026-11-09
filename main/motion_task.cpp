#include "motion_task.h"
#include "joint_driver.h"
#include "arm_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

    ServoTargets mapJointPositionsToServoTargets(const JointPositions& currentPositions, const ServoCalibrationArray& calibrations){
        ServoTargets targets{};

        targets.positionSteps[static_cast<std::size_t>(ServoIdx::Base)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Base)], calibrations[static_cast<std::size_t>(ServoIdx::Base)]);
        targets.positionSteps[static_cast<std::size_t>(ServoIdx::ShoulderA)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Shoulder)], calibrations[static_cast<std::size_t>(ServoIdx::ShoulderA)]);
        targets.positionSteps[static_cast<std::size_t>(ServoIdx::ShoulderB)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Shoulder)], calibrations[static_cast<std::size_t>(ServoIdx::ShoulderB)]);
        targets.positionSteps[static_cast<std::size_t>(ServoIdx::Elbow)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Elbow)], calibrations[static_cast<std::size_t>(ServoIdx::Elbow)]);
        targets.positionSteps[static_cast<std::size_t>(ServoIdx::Gripper)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Gripper)], calibrations[static_cast<std::size_t>(ServoIdx::Gripper)]);

        return targets;
    }

    esp_err_t runMotionCycle(JointPositions& currentPositions, const JointPositions& targetPositions, const JointLimitsArray& limits, const ServoCalibrationArray& calibrations, float deltaTimeSeconds, std::uint16_t speedStepsPerSecond){
        JointPositions nextPositions = updateJointPositions(currentPositions, targetPositions, limits, deltaTimeSeconds);
        ServoTargets servoTargets = mapJointPositionsToServoTargets(nextPositions, calibrations);
        esp_err_t writeResult = st3215::synchronizedWrite(servoTargets, calibrations, speedStepsPerSecond);
        if (writeResult == ESP_OK){
            currentPositions = nextPositions;
        }
        return writeResult;
    }

    static void motionTask(void* param){
        (void)param; //unused

        JointPositions curPositions{};
        JointPositions targetPositions{};


        //initilized the position of the arm to the default positions
        for (std::size_t i = 0; i < static_cast<std::size_t>(JointId::Count); i++){
            curPositions.jointDegrees[i] = arm_config::kJointLimits[i].defaultDegrees;
            targetPositions.jointDegrees[i] = arm_config::kJointLimits[i].defaultDegrees;
        }
        
        TickType_t lastWakeTime = xTaskGetTickCount();

        while(true){
            runMotionCycle(curPositions, targetPositions, arm_config::kJointLimits, arm_config::kServoCalibrations, arm_config::kMotionPeriodSeconds, arm_config::kOperatingSpeedStepsPerSecond);
            vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(arm_config::kMotionPeriodMS));
        }
    }

    esp_err_t startMotionTask(){
        BaseType_t taskResult = xTaskCreate(
            motionTask,
            "arm_motion",
            4096,
            nullptr,
            5,
            nullptr
        );
        if (taskResult != pdPASS){
            return ESP_FAIL;
        }
        return ESP_OK;
    }

}

