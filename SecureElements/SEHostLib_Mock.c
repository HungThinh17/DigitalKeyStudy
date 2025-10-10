#include "SEHostLib_Mock.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define MOCK_LOG(fmt, ...) fprintf(stderr, "[SEHostMock] " fmt "\n", ##__VA_ARGS__)

SEHostStatus SEHost_Init(void) {
    MOCK_LOG("init");
    return SEHOST_OK;
}

SEHostStatus SEHost_Deinit(void) {
    MOCK_LOG("deinit");
    return SEHOST_OK;
}

SEHostStatus SEHost_PowerOn(void) {
    MOCK_LOG("power on");
    return SEHOST_OK;
}

SEHostStatus SEHost_PowerOff(void) {
    MOCK_LOG("power off");
    return SEHOST_OK;
}

SEHostStatus SEHost_Reset(void) {
    MOCK_LOG("reset");
    return SEHOST_OK;
}

/* Very dumb APDU simulator */
SEHostStatus SEHost_Transmit(const uint8_t *tx,
                             uint32_t tx_len,
                             uint8_t **out_rx,
                             uint32_t *out_rx_len,
                             uint32_t timeout_ms) {
    if (!tx || !out_rx || !out_rx_len) return SEHOST_ERR_PARAM;

    (void)timeout_ms;
    uint8_t ins = tx[1];
    MOCK_LOG("Transmit INS=0x%02X, len=%u", ins, tx_len);

    if (ins == 0xA4) { // SELECT
        uint8_t mock[] = { 0xDE,0xAD,0xBE,0xEF, 0x90,0x00 };
        *out_rx_len = sizeof(mock);
        *out_rx = malloc(*out_rx_len);
        memcpy(*out_rx, mock, *out_rx_len);
        usleep(20 * 1000);
    } else if (ins == 0x30 || ins == 0x32) { // SPAKE2
        uint8_t mock[] = { 0x11,0x22,0x33, 0x90,0x00 };
        *out_rx_len = sizeof(mock);
        *out_rx = malloc(*out_rx_len);
        memcpy(*out_rx, mock, *out_rx_len);
        usleep(50 * 1000);
    } else if (ins == 0xD4 || ins == 0x3C) { // WRITE / OP_CONTROL
        uint8_t mock[] = { 0x90, 0x00 };
        *out_rx_len = sizeof(mock);
        *out_rx = malloc(*out_rx_len);
        memcpy(*out_rx, mock, *out_rx_len);
        usleep(10 * 1000);
    } else if (ins == 0xCA || ins == 0xC0) { // GET DATA
        uint8_t mock[] = { 0xAA,0xBB,0xCC, 0x90,0x00 };
        *out_rx_len = sizeof(mock);
        *out_rx = malloc(*out_rx_len);
        memcpy(*out_rx, mock, *out_rx_len);
    } else {
        uint8_t mock[] = { 0x6A,0x81 }; // Not supported
        *out_rx_len = sizeof(mock);
        *out_rx = malloc(*out_rx_len);
        memcpy(*out_rx, mock, *out_rx_len);
    }

    return SEHOST_OK;
}
