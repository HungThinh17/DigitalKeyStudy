/* SeInCarService.c - In-Car Secure Element Service (using SEHostLib_Mock)
 * All original logic preserved. Internal mock removed.
 */
#include "SeInCarServices.h"
#include "SEHostLib_Mock.h"   /* mock or real host library */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

/* ---------- Configuration ---------- */
#define SE_DEFAULT_TIMEOUT_MS 3000
#define SE_MAX_SESSIONS 8

/* ---------- Internal context ---------- */
typedef struct {
    int initialized;
    int powered;
    pthread_mutex_t lock;
    SeEventCallback_t cb;
    void *cb_ctx;
    SeSessionHandle_t next_session;
    int session_alloc[SE_MAX_SESSIONS];
} SeContext_t;

static SeContext_t g_ctx = {
    .initialized = 0,
    .powered = 0,
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .cb = NULL,
    .cb_ctx = NULL,
    .next_session = 1
};

/* ---------- Logging helper ---------- */
#define SE_LOG(fmt, ...) do { fprintf(stderr, "[SE] " fmt "\n", ##__VA_ARGS__); } while(0)

/* ---------- Internal helpers ---------- */
static int ctx_check_init(void) { return g_ctx.initialized; }
static void ctx_lock(void) { pthread_mutex_lock(&g_ctx.lock); }
static void ctx_unlock(void) { pthread_mutex_unlock(&g_ctx.lock); }

/* allocate session */
static SeSessionHandle_t ctx_alloc_session(void) {
    ctx_lock();
    for (int i = 0; i < SE_MAX_SESSIONS; ++i) {
        if (!g_ctx.session_alloc[i]) {
            g_ctx.session_alloc[i] = 1;
            SeSessionHandle_t h = g_ctx.next_session++;
            if (g_ctx.next_session == 0) g_ctx.next_session = 1;
            ctx_unlock();
            return h;
        }
    }
    ctx_unlock();
    return SE_SESSION_INVALID;
}

/* free session */
static void ctx_free_session(SeSessionHandle_t s) {
    ctx_lock();
    for (int i = 0; i < SE_MAX_SESSIONS; ++i) {
        if (g_ctx.session_alloc[i]) {
            g_ctx.session_alloc[i] = 0;
            break;
        }
    }
    ctx_unlock();
}

/* Build + send APDU through SEHost, parse response */
static SeStatus_t se_send_apdu_internal(const ApduFrame *frame, ApduResponse *out_resp, uint32_t timeout_ms) {
    if (!frame || !out_resp) return SE_ERR_INVALID_PARAM;

    ApduWireBuffer wire = {0};
    if (apdu_serialize_frame(frame, &wire) != 0) {
        return SE_ERR_NOMEM;
    }

    uint8_t *rx = NULL;
    uint32_t rxlen = 0;

    SEHostStatus rc = SEHost_Transmit(wire.buf, wire.len, &rx, &rxlen, timeout_ms);
    apdu_free_wirebuffer(&wire);

    if (rc != SEHOST_OK) {
        if (rx) free(rx);
        return SE_ERR_COMM;
    }

    if (apdu_parse_response(rx, rxlen, out_resp) != 0) {
        free(rx);
        return SE_ERR_COMM;
    }
    free(rx);
    return SE_OK;
}

/* Convert typed param -> ApduGenericParam */
static int convert_param_to_generic(ApduCommandId cmd, const void *param, ApduGenericParam *out) {
    if (!out) return -1;
    out->data = NULL;
    out->data_len = 0;
    out->le = 0;
    out->cla_override = 0xFF;

    if (!param) return 0;

    switch (cmd) {
        case APDU_CMD_SELECT: {
            const SeSelectParam_t *p = (const SeSelectParam_t*)param;
            if (p && p->aid && p->aid_len) {
                out->data = p->aid;
                out->data_len = p->aid_len;
            }
            break;
        }
        case APDU_CMD_SPAKE2_REQUEST:
        case APDU_CMD_SPAKE2_VERIFY: {
            const SeSpakeMsgParam_t *p = (const SeSpakeMsgParam_t*)param;
            if (!p || !p->msg || p->msg_len == 0) return -1;
            out->data = p->msg;
            out->data_len = p->msg_len;
            break;
        }
        case APDU_CMD_WRITE_DATA: {
            const SeWriteDataParam_t *p = (const SeWriteDataParam_t*)param;
            if (!p || !p->payload || p->payload_len == 0) return -1;
            out->data = p->payload;
            out->data_len = p->payload_len;
            break;
        }
        case APDU_CMD_OP_CONTROL_FLOW: {
            const SeOpControlParam_t *p = (const SeOpControlParam_t*)param;
            if (p && p->meta && p->meta_len) {
                out->data = p->meta;
                out->data_len = p->meta_len;
            } else {
                out->data = &p->control;
                out->data_len = 1;
            }
            break;
        }
        default: {
            const SeRawApduParam_t *p = (const SeRawApduParam_t*)param;
            if (p && p->raw && p->raw_len) {
                out->data = p->raw;
                out->data_len = p->raw_len;
                out->le = p->le;
            }
            break;
        }
    }
    return 0;
}

/* ---------- API Implementation ---------- */

SeStatus_t Se_Init(SeEventCallback_t cb, void *event_ctx) {
    ctx_lock();
    if (g_ctx.initialized) {
        ctx_unlock();
        return SE_ERR_ALREADY_INIT;
    }
    if (SEHost_Init() != SEHOST_OK) {
        ctx_unlock();
        return SE_ERR_HW;
    }
    g_ctx.cb = cb;
    g_ctx.cb_ctx = event_ctx;
    g_ctx.initialized = 1;
    for (int i = 0; i < SE_MAX_SESSIONS; ++i) g_ctx.session_alloc[i] = 0;
    ctx_unlock();
    SE_LOG("Service initialized");
    return SE_OK;
}

void Se_Deinit(void) {
    ctx_lock();
    if (g_ctx.initialized) {
        SEHost_Deinit();
        g_ctx.initialized = 0;
        g_ctx.cb = NULL;
        g_ctx.cb_ctx = NULL;
    }
    ctx_unlock();
}

SeStatus_t Se_PowerOn(void) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    ctx_lock();
    if (g_ctx.powered) { ctx_unlock(); return SE_OK; }
    if (SEHost_PowerOn() != SEHOST_OK) { ctx_unlock(); return SE_ERR_HW; }
    g_ctx.powered = 1;
    ctx_unlock();
    if (g_ctx.cb) g_ctx.cb(SE_EVENT_POWERED_ON, g_ctx.cb_ctx, NULL);
    return SE_OK;
}

SeStatus_t Se_PowerOff(void) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    ctx_lock();
    if (!g_ctx.powered) { ctx_unlock(); return SE_OK; }
    if (SEHost_PowerOff() != SEHOST_OK) { ctx_unlock(); return SE_ERR_HW; }
    g_ctx.powered = 0;
    ctx_unlock();
    if (g_ctx.cb) g_ctx.cb(SE_EVENT_POWERED_OFF, g_ctx.cb_ctx, NULL);
    return SE_OK;
}

SeStatus_t Se_Reset(void) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    if (SEHost_Reset() != SEHOST_OK) return SE_ERR_HW;
    if (g_ctx.cb) g_ctx.cb(SE_EVENT_RESET, g_ctx.cb_ctx, NULL);
    return SE_OK;
}

SeStatus_t Se_DetectApplet(uint8_t *applet_version_buf, uint16_t *ver_len) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    ApduFrame frame;
    memset(&frame, 0, sizeof(frame));
    if (apdu_build_command(APDU_CMD_SELECT, NULL, &frame) != 0) return SE_ERR_UNKNOWN;

    ApduResponse resp;
    SeStatus_t st = se_send_apdu_internal(&frame, &resp, SE_DEFAULT_TIMEOUT_MS);
    apdu_free_frame(&frame);
    if (st != SE_OK) return st;

    if (resp.sw1 == 0x90 && resp.sw2 == 0x00) {
        if (applet_version_buf && ver_len) {
            uint16_t copy_len = (*ver_len < resp.len) ? *ver_len : (uint16_t)resp.len;
            if (copy_len > 0 && resp.data) memcpy(applet_version_buf, resp.data, copy_len);
            *ver_len = copy_len;
        }
        apdu_free_response(&resp);
        if (g_ctx.cb) g_ctx.cb(SE_EVENT_APPLET_DETECTED, g_ctx.cb_ctx, NULL);
        return SE_OK;
    } else {
        apdu_free_response(&resp);
        return SE_ERR_NO_APPLET;
    }
}

SeStatus_t Se_OpenSession(SeSessionHandle_t *out_session) {
    if (!ctx_check_init() || !out_session) return SE_ERR_NOT_INIT;
    SeSessionHandle_t h = ctx_alloc_session();
    if (h == SE_SESSION_INVALID) return SE_ERR_BUSY;
    *out_session = h;
    return SE_OK;
}

SeStatus_t Se_CloseSession(SeSessionHandle_t session) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    if (session == SE_SESSION_INVALID) return SE_ERR_INVALID_PARAM;
    ctx_free_session(session);
    return SE_OK;
}

/* Synchronous send */
SeStatus_t Se_SendApduSync(SeSessionHandle_t session, ApduCommandId cmd, const void *param, ApduResponse *resp, uint32_t timeout_ms) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    if (!g_ctx.powered) return SE_ERR_HW;
    if (!resp) return SE_ERR_INVALID_PARAM;
    (void)session;

    ApduFrame frame;
    memset(&frame, 0, sizeof(frame));
    ApduGenericParam gparam;
    if (convert_param_to_generic(cmd, param, &gparam) != 0) return SE_ERR_INVALID_PARAM;

    if (apdu_build_command(cmd, &gparam, &frame) != 0) return SE_ERR_UNKNOWN;

    SeStatus_t st = se_send_apdu_internal(&frame, resp, timeout_ms ? timeout_ms : SE_DEFAULT_TIMEOUT_MS);
    apdu_free_frame(&frame);
    return st;
}

/* Async worker */
typedef struct {
    SeSessionHandle_t session;
    ApduCommandId cmd;
    void *param;
    SeApduAsyncCb_t cb;
    void *user_ctx;
    uint32_t timeout_ms;
} AsyncJob_t;

static void *async_worker_thread(void *arg) {
    AsyncJob_t job = *(AsyncJob_t*)arg;
    free(arg);

    ApduResponse *resp = malloc(sizeof(ApduResponse));
    if (!resp) {
        if (job.cb) job.cb(job.session, job.cmd, NULL, job.user_ctx);
        return NULL;
    }
    memset(resp, 0, sizeof(ApduResponse));

    SeStatus_t st = Se_SendApduSync(job.session, job.cmd, job.param, resp, job.timeout_ms);
    (void)st;
    if (job.cb) job.cb(job.session, job.cmd, resp, job.user_ctx);

    apdu_free_response(resp);
    free(resp);
    return NULL;
}

SeStatus_t Se_SendApduAsync(SeSessionHandle_t session, ApduCommandId cmd, const void *param, SeApduAsyncCb_t user_cb, void *user_ctx, uint32_t timeout_ms) {
    if (!ctx_check_init()) return SE_ERR_NOT_INIT;
    if (!g_ctx.powered) return SE_ERR_HW;
    if (!user_cb) return SE_ERR_INVALID_PARAM;

    AsyncJob_t *job = malloc(sizeof(AsyncJob_t));
    if (!job) return SE_ERR_NOMEM;
    job->session = session;
    job->cmd = cmd;
    job->param = (void*)param;
    job->cb = user_cb;
    job->user_ctx = user_ctx;
    job->timeout_ms = timeout_ms ? timeout_ms : SE_DEFAULT_TIMEOUT_MS;

    pthread_t t;
    if (pthread_create(&t, NULL, async_worker_thread, job) != 0) {
        free(job);
        return SE_ERR_UNKNOWN;
    }
    pthread_detach(t);
    return SE_OK;
}

/* High-level wrappers */
SeStatus_t Se_SelectFramework(SeSessionHandle_t session, const SeSelectParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_SELECT, param, resp, timeout_ms);
}
SeStatus_t Se_Spake2Request(SeSessionHandle_t session, const SeSpakeMsgParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_SPAKE2_REQUEST, param, resp, timeout_ms);
}
SeStatus_t Se_Spake2Verify(SeSessionHandle_t session, const SeSpakeMsgParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_SPAKE2_VERIFY, param, resp, timeout_ms);
}
SeStatus_t Se_WriteData(SeSessionHandle_t session, const SeWriteDataParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_WRITE_DATA, param, resp, timeout_ms);
}
SeStatus_t Se_OpControlFlow(SeSessionHandle_t session, const SeOpControlParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_OP_CONTROL_FLOW, param, resp, timeout_ms);
}
SeStatus_t Se_GetData(SeSessionHandle_t session, const SeRawApduParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_GET_DATA, param, resp, timeout_ms);
}
SeStatus_t Se_PresenceCheck(SeSessionHandle_t session, const SePresenceParam_t *param, ApduResponse *resp, uint32_t timeout_ms) {
    return Se_SendApduSync(session, APDU_CMD_PRESENCE0, param, resp, timeout_ms);
}

#endif /* SeInCarService.c */
