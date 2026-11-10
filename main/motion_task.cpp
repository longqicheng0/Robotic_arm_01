#include "motion_task.h"

#include <cstddef>

#include "arm_config.h"
#include "arm_control.h"
#include "joint_driver.h"
#include "servo_config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace arm
{
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
            runMotionCycle(curPositions, targetPositions, arm_config::kJointLimits, servo_config::kServoCalibrations, arm_config::kMotionPeriodSeconds, servo_config::kOperatingSpeedStepsPerSecond);
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

