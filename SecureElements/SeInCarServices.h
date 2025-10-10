#ifndef SE_IN_CAR_SERVICE_H
#define SE_IN_CAR_SERVICE_H

#include <stdint.h>
#include "ApduTypes.h"      /* ApduFrame, ApduResponse */
#include "ApduUtilities.h"  /* apdu_build_command, apdu_serialize_frame, apdu_parse_response */

#ifdef __cplusplus
extern "C" {
#endif

/* ---------- Status / errors ---------- */
typedef enum {
    SE_OK = 0,
    SE_ERR_NOT_INIT,
    SE_ERR_ALREADY_INIT,
    SE_ERR_HW,
    SE_ERR_COMM,
    SE_ERR_TIMEOUT,
    SE_ERR_INVALID_PARAM,
    SE_ERR_NO_APPLET,
    SE_ERR_APDU_FAILED,
    SE_ERR_BUSY,
    SE_ERR_NOMEM,
    SE_ERR_UNKNOWN
} SeStatus_t;

/* ---------- Session handle ---------- */
typedef uint32_t SeSessionHandle_t;
#define SE_SESSION_INVALID 0

/* ---------- Events / callback ---------- */
typedef enum {
    SE_EVENT_POWERED_ON,
    SE_EVENT_POWERED_OFF,
    SE_EVENT_RESET,
    SE_EVENT_APPLET_DETECTED,
    SE_EVENT_APPLET_REMOVED,
    SE_EVENT_APDU_RESPONSE_AVAILABLE, /* for async deliveries */
    SE_EVENT_ERROR
} SeEvent_t;

typedef void (*SeEventCallback_t)(SeEvent_t ev, void *event_ctx, void *payload);

/* ---------- APDU param structs (typed) ----------
   These are the logical (not wire-TLV) structs callers use.
   The service will serialize them into TLV blobs passed to apdu_build_command.
*/

/* SELECT */
typedef struct {
    const uint8_t *aid;      /* if NULL, default CCC Framework AID used */
    uint16_t aid_len;
    uint8_t p1;
    uint8_t p2;
} SeSelectParam_t;

/* SPAKE2 request/verify (opaque bytes) */
typedef struct {
    const uint8_t *msg;      /* SPAKE2 TLV payload built by caller */
    uint32_t msg_len;
} SeSpakeMsgParam_t;

/* WRITE DATA: arbitrary TLV blob (for key material, certs, etc.) */
typedef struct {
    const uint8_t *payload;
    uint32_t payload_len;
    uint8_t chunk_index;     /* optional for multi-chunk writes */
    uint8_t final_chunk;     /* 0/1 */
} SeWriteDataParam_t;

/* OP CONTROL FLOW */
typedef struct {
    uint8_t control;         /* e.g. 0x00 = finalize, 0x01 = abort */
    const uint8_t *meta;     /* optional additional info */
    uint16_t meta_len;
} SeOpControlParam_t;

/* PRESENCE/AUTH simple payload (some spec-defined small payloads) */
typedef struct {
    const uint8_t *nonce;
    uint16_t nonce_len;
} SePresenceParam_t;

/* Generic fallback (raw TLV) */
typedef struct {
    const uint8_t *raw;
    uint32_t raw_len;
    uint32_t le;             /* expected response length override */
} SeRawApduParam_t;

/* ---------- Public API ---------- */

/* Initialize service. Callback is optional (can be NULL). */
SeStatus_t Se_Init(SeEventCallback_t cb, void *event_ctx);

/* Deinitialize service */
void Se_Deinit(void);

/* Power and reset control */
SeStatus_t Se_PowerOn(void);
SeStatus_t Se_PowerOff(void);
SeStatus_t Se_Reset(void);

/* Detect applet: fills in optional info (if detected). Returns SE_OK if applet present. */
SeStatus_t Se_DetectApplet(/* out */ uint8_t *applet_version_buf, uint16_t *ver_len);

/* Session management */
SeStatus_t Se_OpenSession(/* out */ SeSessionHandle_t *out_session);
SeStatus_t Se_CloseSession(SeSessionHandle_t session);

/* Low-level APDU send (sync). The param must be pointer to the corresponding typed struct,
   or SeRawApduParam_t for arbitrary TLVs. On success the caller owns resp->data and must call apdu_free_response(). */
SeStatus_t Se_SendApduSync(SeSessionHandle_t session,
                           ApduCommandId cmd,
                           const void *param,
                           ApduResponse *resp,
                           uint32_t timeout_ms);

/* Low-level APDU send (async). user_cb is invoked on completion with a copy of the ApduResponse allocated by service.
   Caller must not free that copy. The payload delivers ApduResponse* pointer which is owned by the callback until
   callback returns; service will free it after callback returns. */
typedef void (*SeApduAsyncCb_t)(SeSessionHandle_t session, ApduCommandId cmd, ApduResponse *resp, void *user_ctx);
SeStatus_t Se_SendApduAsync(SeSessionHandle_t session,
                            ApduCommandId cmd,
                            const void *param,
                            SeApduAsyncCb_t user_cb,
                            void *user_ctx,
                            uint32_t timeout_ms);

/* Convenience high-level functions that mirror the CCC owner-pairing happy path */
SeStatus_t Se_SelectFramework(SeSessionHandle_t session, const SeSelectParam_t *param, ApduResponse *resp, uint32_t timeout_ms);
SeStatus_t Se_Spake2Request(SeSessionHandle_t session, const SeSpakeMsgParam_t *param, ApduResponse *resp, uint32_t timeout_ms);
SeStatus_t Se_Spake2Verify(SeSessionHandle_t session, const SeSpakeMsgParam_t *param, ApduResponse *resp, uint32_t timeout_ms);
SeStatus_t Se_WriteData(SeSessionHandle_t session, const SeWriteDataParam_t *param, ApduResponse *resp, uint32_t timeout_ms);
SeStatus_t Se_OpControlFlow(SeSessionHandle_t session, const SeOpControlParam_t *param, ApduResponse *resp, uint32_t timeout_ms);
SeStatus_t Se_GetData(SeSessionHandle_t session, const SeRawApduParam_t *param, ApduResponse *resp, uint32_t timeout_ms);
SeStatus_t Se_PresenceCheck(SeSessionHandle_t session, const SePresenceParam_t *param, ApduResponse *resp, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* SE_IN_CAR_SERVICE_H */
