#include <stdint.h>
#include <stddef.h>

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

    int bytesWritten = uart_write_bytes(
        arm_config::kServoUart,
        packet,
        sizeof(packet)
    );
    if (bytesWritten != 6){return ESP_FAIL;}

    return ESP_ERR_NOT_SUPPORTED;
}
