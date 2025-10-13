#include "SEHostLib_Mock.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   // for usleep

/* ------------------------------------------------------------------
 * Mock logging helper
 * ------------------------------------------------------------------ */
#ifndef MOCK_LOG
#define MOCK_LOG(fmt, ...)  printf("[SEHostMock] " fmt "\n", ##__VA_ARGS__)
#endif

/* ------------------------------------------------------------------
 * Static context
 * ------------------------------------------------------------------ */
static se_apdu_trancsceive_t g_transceive_cb = NULL;
static uint8_t g_initialized = 0;

/* ------------------------------------------------------------------
 * Default mock transceive
 * ------------------------------------------------------------------ */
SEHostStatus se_default_transceive(uint8_t *tx, uint32_t tx_len,
                                   uint8_t *out_rx, uint32_t *out_rx_len)
{
    if (!tx || !out_rx || !out_rx_len)
        return SEHOST_ERR_SEND_FAIL;

    uint8_t ins = tx[1];
    MOCK_LOG("Transmit INS=0x%02X, len=%u", ins, tx_len);

    uint8_t *buf = NULL;
    uint32_t len = 0;

    if (ins == 0xA4) {               /* SELECT */
        static const uint8_t mock[] = { 0xDE,0xAD,0xBE,0xEF, 0x90,0x00 };
        len = sizeof(mock);
        buf = (uint8_t*)malloc(len);
        memcpy(buf, mock, len);
        usleep(20 * 1000);
    } else if (ins == 0x30 || ins == 0x32) { /* SPAKE2 */
        static const uint8_t mock[] = { 0x11,0x22,0x33, 0x90,0x00 };
        len = sizeof(mock);
        buf = (uint8_t*)malloc(len);
        memcpy(buf, mock, len);
        usleep(50 * 1000);
    } else if (ins == 0xD4 || ins == 0x3C) { /* WRITE / OP_CONTROL */
        static const uint8_t mock[] = { 0x90, 0x00 };
        len = sizeof(mock);
        buf = (uint8_t*)malloc(len);
        memcpy(buf, mock, len);
        usleep(10 * 1000);
    } else if (ins == 0xCA || ins == 0xC0) { /* GET DATA */
        static const uint8_t mock[] = { 0xAA,0xBB,0xCC, 0x90,0x00 };
        len = sizeof(mock);
        buf = (uint8_t*)malloc(len);
        memcpy(buf, mock, len);
    } else {
        static const uint8_t mock[] = { 0x6A,0x81 }; /* Not supported */
        len = sizeof(mock);
        buf = (uint8_t*)malloc(len);
        memcpy(buf, mock, len);
    }

    if (!buf)
        return SEHOST_ERR_RECEIVE_FAIL;

    if (len > *out_rx_len)
        len = *out_rx_len;

    memcpy(out_rx, buf, len);
    *out_rx_len = len;
    free(buf);

    return SEHOST_OK;
}

/* ------------------------------------------------------------------
 * Public mock lifecycle
 * ------------------------------------------------------------------ */
SEHostStatus SEHost_Init(se_apdu_trancsceive_t pTransceive)
{
    if (g_initialized)
        return SEHOST_OK;

    g_transceive_cb = (pTransceive) ? pTransceive : se_default_transceive;
    g_initialized = 1;
    MOCK_LOG("Initialized mock SEHost library");
    return SEHOST_OK;
}

SEHostStatus SEHost_Close(void)
{
    if (!g_initialized)
        return SEHOST_ERR_RECEIVE_FAIL;
    g_initialized = 0;
    g_transceive_cb = NULL;
    MOCK_LOG("Mock SEHost closed");
    return SEHOST_OK;
}

SEHostStatus SEHost_Reset(void)
{
    if (!g_initialized)
        return SEHOST_ERR_RECEIVE_FAIL;
    MOCK_LOG("Mock SEHost reset");
    return SEHOST_OK;
}

/* ------------------------------------------------------------------
 * Main Tranceive API
 * ------------------------------------------------------------------ */
SEHostStatus SEHost_Tranceive(const uint8_t *tx, uint32_t tx_len,
                              uint8_t *out_rx, uint32_t *out_rx_len)
{
    if (!g_initialized || !tx || !out_rx || !out_rx_len)
        return SEHOST_ERR_SEND_FAIL;

    if (!g_transceive_cb)
        g_transceive_cb = se_default_transceive;

    return g_transceive_cb((uint8_t*)tx, tx_len, out_rx, out_rx_len);
}
