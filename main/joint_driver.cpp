#include <stdint.h>
#include <stddef.h>

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
