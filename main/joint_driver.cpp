#include "joint_driver.h"

#include <cmath>
#include <cstddef>
#include <cstdint>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "servo_config.h"


static uint8_t st3215_checksum(const uint8_t *bytes, size_t length){

    uint8_t sum = 0;

    for (size_t i = 0; i < length; i++){
        sum += bytes[i];
    }

    return (uint8_t)(~sum);
}

static void st3215_build_ping(uint8_t id, uint8_t packet[6]){
    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = id;
    packet[3] = 0x02;
    packet[4] = 0x01;
    packet[5] = st3215_checksum(&packet[2], 3);
}

//prepare driver - initialize: 1. uart para config | 2. set pin | 3. install driver
esp_err_t st3215::initialize()
{
    uart_config_t config = {};

    config.baud_rate            = servo_config::kServoBaudRate;
    config.data_bits            = UART_DATA_8_BITS;
    config.parity               = UART_PARITY_DISABLE;
    config.stop_bits            = UART_STOP_BITS_1;
    config.flow_ctrl            = UART_HW_FLOWCTRL_DISABLE;
    config.rx_flow_ctrl_thresh  = 0;
    config.source_clk           = UART_SCLK_DEFAULT;

    esp_err_t status = uart_param_config(
        servo_config::kServoUart,
        &config
    );
    if (status != ESP_OK){return status;}

    esp_err_t pinStatus = uart_set_pin(
        servo_config::kServoUart,
        servo_config::kServoTxGpio,
        servo_config::kServoRxGpio,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );
    if (pinStatus != ESP_OK){return pinStatus;}

    esp_err_t driverStatus = uart_driver_install(
        servo_config::kServoUart,
        256, //rx buffer size
        0, //tx buffer size
        0, //queue size
        nullptr, //queue handling
        0 //interrupt flag
    );
    if (driverStatus != ESP_OK){return driverStatus;}

    return ESP_OK;
}

esp_err_t st3215::ping(std::uint8_t id)
{
    std::uint8_t packet[6];

    st3215_build_ping(id,packet);

    esp_err_t clearBytes = uart_flush_input(
        servo_config::kServoUart
    );
    if (clearBytes != ESP_OK){return clearBytes;}

    int bytesWritten = uart_write_bytes(
        servo_config::kServoUart,
        packet,
        sizeof(packet)
    );
    if (bytesWritten != 6){return ESP_FAIL;}

    esp_err_t txStatus = uart_wait_tx_done(
        servo_config::kServoUart,
        pdMS_TO_TICKS(10)
    );
    if (txStatus != ESP_OK){return txStatus;}

    std::uint8_t response[sizeof(packet)];

    int bytesRead = uart_read_bytes(
        servo_config::kServoUart,
        response,
        sizeof(response),
        pdMS_TO_TICKS(20)
    );
    if (bytesRead < 0){return ESP_FAIL;}
    if (bytesRead != sizeof(response)){return ESP_ERR_TIMEOUT;}

    if (   response[0] != 0xFF
        || response[1] != 0xFF
        || response[2] != id
        || response[3] != 0x02
    ){return ESP_ERR_INVALID_RESPONSE;}

    if (response[5] != st3215_checksum(&response[2], 3)){return ESP_ERR_INVALID_CRC;}
    if (response[4] != 0){
        return ESP_FAIL;
    }

    return ESP_OK;

}

std::uint16_t st3215::degreesToPositionSteps(float degrees, const arm::ServoCalibration& calibration)
{
    float stepsPerDegree = 4096.0f / 360.0f;
    float direction = calibration.reversed ? -1.0f : 1.0f;

    float rawSteps = calibration.zeroStep + direction * degrees * stepsPerDegree;

    if (rawSteps < 0){return 0;}
    if (rawSteps > 4095.0f){return 4095.0f;}

    std::uint16_t steps = static_cast<std::uint16_t>(std::lround(rawSteps));

    return steps;

}

arm::ServoTargets arm::mapJointPositionsToServoTargets(const JointPositions& currentPositions, const ServoCalibrationArray& calibrations)
{
    ServoTargets targets{};

    targets.positionSteps[static_cast<std::size_t>(ServoIdx::Base)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Base)], calibrations[static_cast<std::size_t>(ServoIdx::Base)]);
    targets.positionSteps[static_cast<std::size_t>(ServoIdx::ShoulderA)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Shoulder)], calibrations[static_cast<std::size_t>(ServoIdx::ShoulderA)]);
    targets.positionSteps[static_cast<std::size_t>(ServoIdx::ShoulderB)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Shoulder)], calibrations[static_cast<std::size_t>(ServoIdx::ShoulderB)]);
    targets.positionSteps[static_cast<std::size_t>(ServoIdx::Elbow)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Elbow)], calibrations[static_cast<std::size_t>(ServoIdx::Elbow)]);
    targets.positionSteps[static_cast<std::size_t>(ServoIdx::Gripper)] = st3215::degreesToPositionSteps(currentPositions.jointDegrees[static_cast<std::size_t>(JointId::Gripper)], calibrations[static_cast<std::size_t>(ServoIdx::Gripper)]);

    return targets;
}

esp_err_t st3215::writePosition(std::uint8_t id, std::uint16_t positionSteps, std::uint16_t speedStepsPerSecond){
    if (positionSteps > 4095){
        return ESP_ERR_INVALID_ARG;
    }

    std::uint8_t packet[13]{};

    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = id;
    packet[3] = 0x09; //length
    packet[4] = 0x03; //instruction: write
    packet[5] = 0x2A; //target position register address
    packet[6] = static_cast<std::uint8_t>(positionSteps & 0xFF);
    packet[7] = static_cast<std::uint8_t>((positionSteps >> 8) & 0xFF);
    packet[8] = 0;
    packet[9] = 0;
    packet[10] = static_cast<std::uint8_t>(speedStepsPerSecond & 0xFF);
    packet[11] = static_cast<std::uint8_t>((speedStepsPerSecond >> 8) & 0xFF);
    packet[12] = st3215_checksum(&packet[2], 10);

    int bytesWritten = uart_write_bytes(
        servo_config::kServoUart,
        packet,
        sizeof(packet)
    );

    if (bytesWritten != sizeof(packet)){
        return ESP_FAIL;
    }

    esp_err_t txStatus = uart_wait_tx_done(
        servo_config::kServoUart,
        pdMS_TO_TICKS(10)
    );

    if (txStatus != ESP_OK){
        return txStatus;
    }

    return ESP_OK;
}

esp_err_t st3215::readPosition(std::uint8_t id, std::uint16_t& positionSteps){
    std::uint8_t packet[8];

    packet[0] = 0xFF;
    packet[1] = 0xFF;   
    packet[2] = id;
    packet[3] = 0x04; //length
    packet[4] = 0x02; //instruction: read
    packet[5] = 0x38; //register address: present position
    packet[6] = 0x02; //number of bytes to read
    packet[7] = st3215_checksum(&packet[2], 5);

    esp_err_t clearBytes = uart_flush_input(
        servo_config::kServoUart
    );
    if (clearBytes != ESP_OK){return clearBytes;}

    int requestBytes = uart_write_bytes(
        servo_config::kServoUart,
        packet,
        sizeof(packet)
    );
    
    if (requestBytes != sizeof(packet)){return ESP_FAIL;}

    esp_err_t txStatus = uart_wait_tx_done(
        servo_config::kServoUart,
        pdMS_TO_TICKS(10)
    );
    if (txStatus != ESP_OK){return txStatus;}

    uint8_t response[8]; 
    
    int bytesRead = uart_read_bytes(
        servo_config::kServoUart,
        response,
        sizeof(response),
        pdMS_TO_TICKS(20)
    );

    if (bytesRead < 0){return ESP_FAIL;}
    if (bytesRead != sizeof(response)){return ESP_ERR_TIMEOUT;}

    if (   response[0] != 0xFF
        || response[1] != 0xFF
        || response[2] != id
        || response[3] != 0x04
    ){return ESP_ERR_INVALID_RESPONSE;}

    if (response[7] != st3215_checksum(&response[2], 5)){return ESP_ERR_INVALID_CRC;}
    if (response[4] != 0){
        return ESP_FAIL;}

    positionSteps = static_cast<std::uint16_t>(response[5]) | (static_cast<std::uint16_t>(response[6]) << 8);

    return ESP_OK;
}

esp_err_t st3215::synchronizedWrite(const arm::ServoTargets& targets, const arm::ServoCalibrationArray& calibrations, std::uint16_t speedStepsPerSecond){
    std::size_t constexpr byteSize = 7 + 5 * 7 + 1;

    uint8_t packet[byteSize]{};

    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = 0xFE; //broadcast ID
    packet[3] = 7 * 5 + 4; //length: 7 bytes per servo + 4 bytes for instruction, register address, and data length
    packet[4] = 0x83; //instruction: sync write
    packet[5] = 0x2A; //register address: target position
    packet[6] = 0x06; //number of bytes to write per servo 

    for (std::size_t i = 0; i < static_cast<std::size_t>(arm::ServoIdx::Count); i++){
        std::uint8_t servoId = calibrations[i].busId;
        

        std::uint16_t positionSteps = targets.positionSteps[i];
        if (positionSteps > 4095){return ESP_ERR_INVALID_ARG;}

        std::uint16_t speedSteps = speedStepsPerSecond;

        std::size_t offset = 7 + i * 7;

        packet[offset]     = servoId;
        packet[offset + 1] = static_cast<std::uint8_t>(positionSteps & 0xFF);
        packet[offset + 2] = static_cast<std::uint8_t>((positionSteps >> 8) & 0xFF);
        packet[offset + 3] = 0; //reserved
        packet[offset + 4] = 0; //reserved
        packet[offset + 5] = static_cast<std::uint8_t>(speedSteps & 0xFF);
        packet[offset + 6] = static_cast<std::uint8_t>((speedSteps >> 8) & 0xFF);
    }

    packet[byteSize - 1] = st3215_checksum(&packet[2], byteSize - 3);

    // send the packet over UART
    int bytesWritten = uart_write_bytes(
        servo_config::kServoUart,
        packet,
        sizeof(packet)
    );

    if (bytesWritten != sizeof(packet)){
        return ESP_FAIL;
    }

    esp_err_t txStatus = uart_wait_tx_done(
        servo_config::kServoUart,
        pdMS_TO_TICKS(10)
    );
    if (txStatus != ESP_OK){
        return txStatus;
    }

    return ESP_OK;
}
