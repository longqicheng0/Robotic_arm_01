#include <stdint.h>
#include <stddef.h>
#include <cmath>

#include "arm_config.h"
#include "joint_driver.h"


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

    config.baud_rate            = arm_config::kServoBaudRate;
    config.data_bits            = UART_DATA_8_BITS;
    config.parity               = UART_PARITY_DISABLE;
    config.stop_bits            = UART_STOP_BITS_1;
    config.flow_ctrl            = UART_HW_FLOWCTRL_DISABLE;
    config.rx_flow_ctrl_thresh  = 0;
    config.source_clk           = UART_SCLK_DEFAULT;

    esp_err_t status = uart_param_config(
        arm_config::kServoUart,
        &config
    );
    if (status != ESP_OK){return status;}

    esp_err_t pinStatus = uart_set_pin(
        arm_config::kServoUart,
        arm_config::kServoTxGpio,
        arm_config::kServoRxGpio,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    );
    if (pinStatus != ESP_OK){return pinStatus;}

    esp_err_t driverStatus = uart_driver_install(
        arm_config::kServoUart,
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
        arm_config::kServoUart
    );
    if (clearBytes != ESP_OK){return clearBytes;}

    int bytesWritten = uart_write_bytes(
        arm_config::kServoUart,
        packet,
        sizeof(packet)
    );
    if (bytesWritten != 6){return ESP_FAIL;}

    esp_err_t txStatus = uart_wait_tx_done(
        arm_config::kServoUart,
        pdMS_TO_TICKS(10)
    );
    if (txStatus != ESP_OK){return txStatus;}

    std::uint8_t response[sizeof(packet)];

    int bytesRead = uart_read_bytes(
        arm_config::kServoUart,
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
