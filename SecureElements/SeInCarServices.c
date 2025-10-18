
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SeInCarConfig.h"
#include "SeInCarServices.h"

#ifdef MOCK_SE_HOSTLIB
#include "SeInCarServices.h"
#include "SEHostLib_Mock.h"
#else
typedef enum {
    SEHOST_UNKNOW = 0xFFFF,
    SEHOST_OK = 0x9000,
    SEHOST_ERR_SEND_FAIL = 0x7010,
    SEHOST_ERR_RECEIVE_FAIL = 0x7011
} SEHostStatus;
#endif /* MOCK_SE_HOSTLIB */

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
    SEHostStatus st = SEHOST_UNKNOW;

    if (g_ctx.initialized)
        return SE_ERR_ALREADY_INIT;

#ifdef MOCK_SE_HOSTLIB
    st = SEHost_Init(NULL);
#else
    // TODO
#endif /*MOCK_SE_HOSTLIB*/

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

#ifdef MOCK_SE_HOSTLIB
        SEHost_Deinit();
#else
    // TODO
#endif /*MOCK_SE_HOSTLIB*/


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
    SEHostStatus st = SEHOST_UNKNOW;

    if (!g_ctx.initialized)
        return SE_ERR_NOT_INIT;

#ifdef MOCK_SE_HOSTLIB
    st = SEHost_Reset();
#else
    // TODO
#endif /*MOCK_SE_HOSTLIB*/
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
    SEHostStatus st = SEHOST_UNKNOW;

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
    
#ifdef MOCK_SE_HOSTLIB
    st = SEHost_Transmit(wire.buf, wire.len, &rx, &rx_len, timeout_ms);
#else
    // TODO
#endif /*MOCK_SE_HOSTLIB*/
    
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

/* --------------------------------------------------------- */
/* Digital Key Common Services                               */
/* --------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * Utility: copy APDU response payload into SeSpakeBlob / SeShareBlob etc.
 * ------------------------------------------------------------------------- */
static SeStatus_t copy_resp_to_blob(const ApduResponse *resp, uint8_t **out_data, uint32_t *out_len)
{
    if (!resp || !out_data || !out_len) return SE_ERR_INVALID_PARAM;
    if (resp->len == 0 || resp->data == NULL) {
        *out_data = NULL;
        *out_len = 0;
        return SE_OK;
    }

    uint8_t *buf = (uint8_t*)malloc(resp->len);
    if (!buf) return SE_ERR_NOMEM;
    memcpy(buf, resp->data, resp->len);
    *out_data = buf;
    *out_len = resp->len;
    return SE_OK;
}

/* -------------------------------------------------------------------------
 * Provisioning / config
 * ------------------------------------------------------------------------- */
SeStatus_t Se_CheckProvisioningStatus(SeProvisionInfo *status_out)
{
    if (!status_out) return SE_ERR_INVALID_PARAM;
    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_GET_PROVISION_STATUS, NULL, &resp, 2000);
    if (s != SE_OK) {
        return s;
    }

    /* Parse response payload:
     * Expected TLV (example):
     *   Tag 0x01: applet_version (4 bytes)
     *   Tag 0x02: provisioned (1 byte)
     *   Tag 0x03: ursk_present (1 byte)
     *   Tag 0x04: free_owner_slots (1 byte)
     *   Tag 0x05: total_key_slots (1 byte)
     *   Tag 0x06: supported_curves (1 byte)
     *   Tag 0x10: oem_cert_hash (32 bytes)
     *
     * NOTE: adjust parsing according to your ApduUtilities/APDU TLV schema.
     */

    /* Zero the output first */
    memset(status_out, 0, sizeof(*status_out));

    /* Basic/defensive parse: if no data, return not provisioned */
    if (resp.len < 1 || resp.data == NULL) {
        apdu_free_response(&resp);
        return SE_ERR_NOT_PROVISIONED;
    }

    /* Simple heuristic parse: assume fixed layout used by SE */
    const uint8_t *p = resp.data;
    uint32_t remaining = resp.len;

    /* We'll attempt a minimal safe parse: read fields in order if present */
    if (remaining >= 4) {
        /* applet_version */
        status_out->applet_version =
            ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
        p += 4; remaining -= 4;
    }
    if (remaining >= 1) {
        status_out->provisioned = p[0]; p++; remaining--;
    }
    if (remaining >= 1) {
        status_out->ursk_present = p[0]; p++; remaining--;
    }
    if (remaining >= 1) {
        status_out->free_owner_slots = p[0]; p++; remaining--;
    }
    if (remaining >= 1) {
        status_out->total_key_slots = p[0]; p++; remaining--;
    }
    if (remaining >= 1) {
        status_out->supported_curves = p[0]; p++; remaining--;
    }
    if (remaining >= 32) {
        memcpy(status_out->oem_cert_hash, p, 32);
        p += 32; remaining -= 32;
    }

    apdu_free_response(&resp);

    if (!status_out->provisioned) return SE_ERR_NOT_PROVISIONED;
    return SE_OK;
}

SeStatus_t Se_GetProvisioningInfo(SeProvisionInfo *info)
{
    if (!info) return SE_ERR_INVALID_PARAM;
    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_GET_PROVISION_INFO, NULL, &resp, 2000);
    if (s != SE_OK) return s;

    /* For now reuse the same parsing logic as CheckProvisioningStatus, but copy more fields if present */
    memset(info, 0, sizeof(*info));
    if (resp.len >= 4) {
        const uint8_t *p = resp.data;
        uint32_t remaining = resp.len;
        info->applet_version =
            ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
        p += 4; remaining -= 4;
        if (remaining >= 1) { info->provisioned = p[0]; p++; remaining--; }
        if (remaining >= 1) { info->ursk_present = p[0]; p++; remaining--; }
        if (remaining >= 1) { info->free_owner_slots = p[0]; p++; remaining--; }
        if (remaining >= 1) { info->total_key_slots = p[0]; p++; remaining--; }
        if (remaining >= 1) { info->supported_curves = p[0]; p++; remaining--; }
        if (remaining >= 32) { memcpy(info->oem_cert_hash, p, 32); p += 32; remaining -= 32; }
    }

    apdu_free_response(&resp);

    if (!info->provisioned) return SE_ERR_NOT_PROVISIONED;
    return SE_OK;
}

/* -------------------------------------------------------------------------
 * Owner pairing (atomic single-APDU services)
 * ------------------------------------------------------------------------- */

SeStatus_t Se_Pairing_SelectApplet(void)
{
    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_SELECT, NULL, &resp, 1000);
    if (s != SE_OK) return s;
    /* SELECT returning 0x9000 is considered success; Se_SendApduSync already checked SW */
    apdu_free_response(&resp);
    return SE_OK;
}

SeStatus_t Se_Pairing_Spake2Request(const SeSpakeBlob *req, SeSpakeBlob *resp_out)
{
    if (!req || !resp_out) return SE_ERR_INVALID_PARAM;

    /* Build APDU param for SPAKE2_REQUEST
     * ADAPT: use your ApduSpakeParam name if it's different.
     */
    ApduSpakeParam param;
    memset(&param, 0, sizeof(param));
    param.spake_blob = (uint8_t*)req->data;
    param.spake_blob_len = req->len;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_SPAKE2_REQUEST, &param, &resp, 5000);
    if (s != SE_OK) return s;

    /* Copy returned blob to resp_out (caller will free) */
    resp_out->data = NULL;
    resp_out->len = 0;
    if (resp.len > 0 && resp.data) {
        uint8_t *buf = (uint8_t*)malloc(resp.len);
        if (!buf) { apdu_free_response(&resp); return SE_ERR_NOMEM; }
        memcpy(buf, resp.data, resp.len);
        resp_out->data = buf;
        resp_out->len = resp.len;
    }

    apdu_free_response(&resp);
    return SE_OK;
}

SeStatus_t Se_Pairing_Spake2Verify(const SeSpakeBlob *verify, SePairingResult *result_out)
{
    if (!verify || !result_out) return SE_ERR_INVALID_PARAM;

    /* Build APDU param */
    ApduSpakeParam param;
    memset(&param, 0, sizeof(param));
    param.spake_blob = (uint8_t*)verify->data;
    param.spake_blob_len = verify->len;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_SPAKE2_VERIFY, &param, &resp, 5000);

    if (s != SE_OK) return s;

    /* Parse response: assume body contains one byte success flag and optional error code */
    result_out->success = 0;
    result_out->error_code = 0;
    if (resp.len >= 1 && resp.data) {
        result_out->success = resp.data[0] ? 1 : 0;
        if (resp.len >= 2) result_out->error_code = resp.data[1];
    } else {
        /* If no body but SW=9000 then success */
        result_out->success = 1;
    }

    apdu_free_response(&resp);

    if (result_out->success) return SE_OK;
    return SE_ERR_APDU_FAILED;
}

SeStatus_t Se_Pairing_GetStatus(SePairingStatus *status_out)
{
    if (!status_out) return SE_ERR_INVALID_PARAM;
    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_GET_PAIRING_STATUS, NULL, &resp, 1000);
    if (s != SE_OK) return s;

    memset(status_out, 0, sizeof(*status_out));
    if (resp.len >= 1 && resp.data) {
        status_out->paired = resp.data[0] ? 1 : 0;
    }
    if (resp.len >= 2 && resp.data) {
        status_out->locked = resp.data[1] ? 1 : 0;
    }

    apdu_free_response(&resp);
    return SE_OK;
}

/* -------------------------------------------------------------------------
 * Key management / sharing (atomic operations)
 * ------------------------------------------------------------------------- */

SeStatus_t Se_Key_GetInfo(uint8_t key_id, SeKeyInfo *info)
{
    if (!info) return SE_ERR_INVALID_PARAM;
    /* Build simple param containing key id */
    ApduKeyIdParam param;
    memset(&param, 0, sizeof(param));
    param.key_id = key_id;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_KEY_GET_INFO, &param, &resp, 2000);
    if (s != SE_OK) return s;

    memset(info, 0, sizeof(*info));
    if (resp.len >= 1 && resp.data) {
        info->key_id = resp.data[0];
    }
    if (resp.len >= 2 && resp.data) {
        info->valid = resp.data[1];
    }
    if (resp.len >= 3 && resp.data) {
        info->shared = resp.data[2];
    }
    if (resp.len >= 4 && resp.data) {
        info->revoked = resp.data[3];
    }

    apdu_free_response(&resp);
    return SE_OK;
}

SeStatus_t Se_Key_Provision(const SeKeyInfo *info)
{
    if (!info) return SE_ERR_INVALID_PARAM;

    /* Build APDU param (ADAPT: populate correct ApduKeyProvisionParam structure) */
    ApduKeyProvisionParam param;
    memset(&param, 0, sizeof(param));
    param.key_id = info->key_id;
    param.flags = 0;
    if (info->shared) param.flags |= 0x01;
    if (info->revoked) param.flags |= 0x02;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_KEY_PROVISION, &param, &resp, 3000);
    if (s != SE_OK) return s;

    /* treat sw=9000 as success */
    apdu_free_response(&resp);
    return SE_OK;
}

SeStatus_t Se_Key_Revoke(uint8_t key_id)
{
    ApduKeyIdParam param;
    memset(&param, 0, sizeof(param));
    param.key_id = key_id;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_KEY_REVOKE, &param, &resp, 2000);
    if (s != SE_OK) return s;

    apdu_free_response(&resp);
    return SE_OK;
}

SeStatus_t Se_Key_Share(const SeShareBlob *req, SeShareBlob *resp_out)
{
    if (!req || !resp_out) return SE_ERR_INVALID_PARAM;

    ApduBlobParam param;
    memset(&param, 0, sizeof(param));
    param.data = (uint8_t*)req->data;
    param.len = req->len;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_KEY_SHARE, &param, &resp, 5000);
    if (s != SE_OK) return s;

    /* copy response */
    resp_out->data = NULL;
    resp_out->len = 0;
    if (resp.len > 0 && resp.data) {
        uint8_t *buf = (uint8_t*)malloc(resp.len);
        if (!buf) { apdu_free_response(&resp); return SE_ERR_NOMEM; }
        memcpy(buf, resp.data, resp.len);
        resp_out->data = buf;
        resp_out->len = resp.len;
    }

    apdu_free_response(&resp);
    return SE_OK;
}

/* -------------------------------------------------------------------------
 * Authentication / Unlock (atomic)
 * ------------------------------------------------------------------------- */

SeStatus_t Se_Auth_GenerateChallenge(SeChallenge *out)
{
    if (!out) return SE_ERR_INVALID_PARAM;
    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_AUTH_CHALLENGE, NULL, &resp, 1000);
    if (s != SE_OK) return s;

    memset(out, 0, sizeof(*out));
    if (resp.len > 0 && resp.data) {
        uint32_t copy_len = resp.len > sizeof(out->challenge) ? sizeof(out->challenge) : resp.len;
        memcpy(out->challenge, resp.data, copy_len);
        out->len = copy_len;
    } else {
        out->len = 0;
    }

    apdu_free_response(&resp);
    return SE_OK;
}

SeStatus_t Se_Auth_VerifyProof(const SeProof *in, SeAuthResult *result_out)
{
    if (!in || !result_out) return SE_ERR_INVALID_PARAM;

    ApduBlobParam param;
    memset(&param, 0, sizeof(param));
    param.data = (uint8_t*)in->data;
    param.len = in->len;

    ApduResponse resp = {0};
    SeStatus_t s = Se_SendApduSync(APDU_CMD_AUTH_VERIFY, &param, &resp, 2000);
    if (s != SE_OK) return s;

    memset(result_out, 0, sizeof(*result_out));
    if (resp.len >= 1 && resp.data) result_out->verified = resp.data[0] ? 1 : 0;
    if (resp.len >= 2 && resp.data) result_out->error_code = resp.data[1];

    apdu_free_response(&resp);

    return result_out->verified ? SE_OK : SE_ERR_APDU_FAILED;
}

