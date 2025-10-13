#include "SeInCarServices.h"
#include "SEHostLib_Mock.h"
#include <string.h>
#include <stdlib.h>

/* --------------------------------------------------------- */
/* Internal context                                           */
/* --------------------------------------------------------- */
typedef struct {
    uint8_t     initialized;
    uint8_t     powered;
    SeState_t   state;
} SeContext_t;

static SeContext_t g_ctx;

/* --------------------------------------------------------- */
/* Internal helpers                                           */
/* --------------------------------------------------------- */
static SeStatus_t se_convert_host_status(SEHostStatus s)
{
    switch (s) {
        case SEHOST_OK:
            return SE_OK;
        case SEHOST_ERR_SEND_FAIL:
            return SE_ERR_COMM;
        case SEHOST_ERR_RECEIVE_FAIL:
            return SE_ERR_TIMEOUT;
        default:
            return SE_ERR_UNKNOWN;
    }
}

/* --------------------------------------------------------- */
/* Lifecycle                                                  */
/* --------------------------------------------------------- */
SeStatus_t Se_Init(void)
{
    if (g_ctx.initialized)
        return SE_ERR_ALREADY_INIT;

    SEHostStatus st = SEHost_Init(NULL);
    if (st != SEHOST_OK)
        return se_convert_host_status(st);

    memset(&g_ctx, 0, sizeof(g_ctx));
    g_ctx.initialized = 1;
    g_ctx.state = SE_STATE_IDLE;
    return SE_OK;
}

void Se_Deinit(void)
{
    if (!g_ctx.initialized)
        return;
    SEHost_Deinit();
    memset(&g_ctx, 0, sizeof(g_ctx));
    g_ctx.state = SE_STATE_UNINIT;
}

SeStatus_t Se_PowerOn(void)
{
    if (!g_ctx.initialized)
        return SE_ERR_NOT_INIT;

    // Do Power On

    g_ctx.powered = 1;
    return SE_OK;
}

SeStatus_t Se_PowerOff(void)
{
    if (!g_ctx.initialized)
        return SE_ERR_NOT_INIT;

    // Do Power Off

    g_ctx.powered = 0;
    return SE_OK;
}

SeStatus_t Se_Reset(void)
{
    if (!g_ctx.initialized)
        return SE_ERR_NOT_INIT;

    SEHostStatus st = SEHost_Reset();
    return se_convert_host_status(st);
}

/* --------------------------------------------------------- */
/* Access + state                                             */
/* --------------------------------------------------------- */
SeStatus_t Se_IsReady(void)
{
    if (!g_ctx.initialized) return SE_ERR_NOT_INIT;
    if (!g_ctx.powered)     return SE_ERR_HW;
    if (g_ctx.state == SE_STATE_BUSY) return SE_ERR_BUSY;
    return SE_OK;
}

SeState_t Se_GetState(void)
{
    return g_ctx.state;
}

SeStatus_t Se_BeginAccess(void)
{
    SeStatus_t st = Se_IsReady();
    if (st != SE_OK)
        return st;
    g_ctx.state = SE_STATE_BUSY;
    return SE_OK;
}

SeStatus_t Se_EndAccess(void)
{
    if (!g_ctx.initialized)
        return SE_ERR_NOT_INIT;
    g_ctx.state = SE_STATE_IDLE;
    return SE_OK;
}

/* --------------------------------------------------------- */
/* Core APDU transmit                                         */
/* --------------------------------------------------------- */
SeStatus_t Se_SendApduSync(ApduCommandId cmd, const void *param,
                           ApduResponse *resp, uint32_t timeout_ms)
{
    if (Se_IsReady() != SE_OK)
        return SE_ERR_BUSY;
    if (!resp)
        return SE_ERR_INVALID_PARAM;

    g_ctx.state = SE_STATE_BUSY;

    /* Build APDU */
    ApduFrame frame = {0};
    if (apdu_build_command(cmd, param, &frame) != 0) {
        g_ctx.state = SE_STATE_IDLE;
        return SE_ERR_INVALID_PARAM;
    }

    /* Serialize */
    ApduWireBuffer wire = {0};
    int rc = apdu_serialize_frame(&frame, &wire);
    apdu_free_frame(&frame);
    if (rc != 0) {
        g_ctx.state = SE_STATE_IDLE;
        return SE_ERR_INVALID_PARAM;
    }

    /* Send */
    uint8_t *rx = NULL;
    uint32_t rx_len = 0;
    SEHostStatus st = SEHost_Transmit(wire.buf, wire.len, &rx, &rx_len, timeout_ms);
    apdu_free_wirebuffer(&wire);

    if (st != SEHOST_OK) {
        g_ctx.state = SE_STATE_ERROR;
        return se_convert_host_status(st);
    }

    /* Parse response */
    rc = apdu_parse_response(rx, rx_len, resp);
    if (rx) {
        memset(rx, 0, rx_len);
        free(rx);
    }

    if (rc != 0) {
        g_ctx.state = SE_STATE_ERROR;
        return SE_ERR_APDU_FAILED;
    }

    if (resp->sw1 != 0x90 || resp->sw2 != 0x00) {
        g_ctx.state = SE_STATE_ERROR;
        return SE_ERR_APDU_FAILED;
    }

    g_ctx.state = SE_STATE_IDLE;
    return SE_OK;
}
