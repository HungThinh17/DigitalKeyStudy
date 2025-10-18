#ifndef APDU_UTILITIES_H
#define APDU_UTILITIES_H

#include <stdint.h>
#include "ApduTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APDU_CMD_SELECT = 0,
    APDU_CMD_GENERIC,
    APDU_CMD_SPAKE2_REQUEST,
    APDU_CMD_SPAKE2_VERIFY,
    APDU_CMD_WRITE_DATA,
    APDU_CMD_GET_DATA,
    APDU_CMD_GET_RESPONSE,
    APDU_CMD_OP_CONTROL_FLOW,
    APDU_CMD_CREATE_ENDPOINT,
    APDU_CMD_SETUP_ENDPOINT,
    APDU_CMD_AUTHORIZE_ENDPOINT,
    APDU_CMD_SETUP_INSTANCE,
    APDU_CMD_TERMINATE_ENDPOINT,
    APDU_CMD_DELETE_ENDPOINT,
    APDU_CMD_CONVERT_ENDPOINT,
    APDU_CMD_CREATE_ENCRYPTION_KEY,
    APDU_CMD_CREATE_RANGING_KEY,
    APDU_CMD_DELETE_RANGING_KEYS,
    APDU_CMD_SIGN,
    APDU_CMD_GET_PRIVATE_DATA,
    APDU_CMD_SET_PRIVATE_DATA,
    APDU_CMD_SET_CONFIDENTIAL_DATA,
    APDU_CMD_WRITE_BUFFER,
    APDU_CMD_READ_BUFFER,
    APDU_CMD_AUTH0,
    APDU_CMD_AUTH1,
    APDU_CMD_PRESENCE0,
    APDU_CMD_PRESENCE1,
    APDU_CMD_EXCHANGE,
    APDU_CMD_GET_NOTIFICATION,

    /* --- Extended CCC SEInCar commands (from CCC Digital Key TS v4.0) --- */
    APDU_CMD_GET_PROVISION_STATUS,
    APDU_CMD_GET_PROVISION_INFO,
    APDU_CMD_GET_PAIRING_STATUS,
    APDU_CMD_KEY_GET_INFO,
    APDU_CMD_KEY_PROVISION,
    APDU_CMD_KEY_REVOKE,
    APDU_CMD_KEY_SHARE,
    APDU_CMD_AUTH_CHALLENGE,
    APDU_CMD_AUTH_VERIFY,

    APDU_CMD_COUNT
} ApduCommandId;

/* Public API */
/* Build command: now accepts void* param which must point to the right typed struct
   based on 'cmd'. If param == NULL many commands use defaults (e.g., SELECT default AID) */
int apdu_build_command(ApduCommandId cmd, const void *param, ApduFrame *out);

/* Serialize frame to ISO-7816 wire bytes (short & extended APDU support) */
int apdu_serialize_frame(const ApduFrame *frame, ApduWireBuffer *wire);
void apdu_free_wirebuffer(ApduWireBuffer *wire);

/* Parse response */
int apdu_parse_response(const uint8_t *raw, uint32_t len, ApduResponse *resp);
void apdu_free_response(ApduResponse *resp);

/* Free frame */
void apdu_free_frame(ApduFrame *frame);

/* Debugging */
void apdu_dump_wire(const uint8_t *buf, uint32_t len);

/* DK_APDU_RQ wrappers for BLE/UWB encapsulation */
int apdu_is_allowed_cla_for_dk_apdu(uint8_t cla);
int apdu_wrap_dk_apdu_rq(const ApduWireBuffer *apdu_wire, ApduWireBuffer *out_wrapped);
int apdu_unwrap_dk_apdu_rq(const uint8_t *wrapped, uint32_t len, ApduWireBuffer *out_apdu_wire);

#ifdef __cplusplus
}
#endif

#endif /* APDU_UTILITIES_H */
