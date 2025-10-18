#ifndef APDU_TYPES_H
#define APDU_TYPES_H

#include <stdint.h>
#include <stdlib.h>

/*
 * Typed parameter structs for each APDU command used in CCC Digital Key v4.0.0.
 * These are "logical" parameter containers — apdu_build_command serializes them
 * to the APDU data field according to spec TLV/logical format.
 *
 * If a command is marked "opaque payload", the struct carries (payload, len).
 * For commands with small control fields, we model them explicitly.
 */

/* Basic frame/response types */
typedef struct {
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    uint8_t *data;    /* allocated payload (C-APDU data) */
    uint32_t lc;      /* length of payload */
    uint32_t le;      /* expected response length (0 = not present) */
} ApduFrame;

typedef struct {
    uint8_t *data;    /* response data (malloc'd) */
    uint32_t len;
    uint8_t sw1;
    uint8_t sw2;
} ApduResponse;

/* Generic fallback param (if you don't want dedicated struct) */
typedef struct {
    const uint8_t *data;
    uint32_t data_len;
    uint32_t le;           /* 0 = not set */
    uint8_t cla_override;  /* 0xFF = no override */
} ApduGenericParam;

/* SELECT */
typedef struct {
    const uint8_t *aid; /* optional: if NULL use default CCC framework AID */
    uint16_t aid_len;
    uint8_t p1; /* use 0x04 or as caller wants */
    uint8_t p2;
} ApduSelectParam;

/* SPAKE2+ REQUEST and VERIFY (opaque SPAKE blobs) */
typedef struct {
    const uint8_t *spake_blob;
    uint32_t spake_blob_len;
} ApduSpakeParam;

/* WRITE DATA - generic TLV payload (e.g., key creation data). */
typedef struct {
    const uint8_t *tlv;
    uint32_t tlv_len;
    uint32_t offset; /* for chunked writes if needed (0 default) */
} ApduWriteDataParam;

/* GET DATA - request a specific tag or TLV */
typedef struct {
    uint16_t tag;   /* logical tag id (per spec table) */
    uint8_t p1;
    uint8_t p2;
    uint32_t le;    /* expected length */
} ApduGetDataParam;

/* GET RESPONSE - classic Le-only */
typedef struct {
    uint32_t le;
} ApduGetResponseParam;

/* OP CONTROL FLOW */
typedef struct {
    uint8_t opcode; /* e.g. 0x00 = finalize, 0x01 = abort, etc. per spec */
    const uint8_t *payload;
    uint32_t payload_len;
} ApduOpControlFlowParam;

/* Endpoint management commands: CREATE / SETUP / AUTHORIZE / TERMINATE / DELETE / CONVERT */
typedef struct {
    uint8_t endpoint_id;
    const uint8_t *params;
    uint32_t params_len;
} ApduEndpointParam;

typedef struct {
    uint8_t instance_id;
    const uint8_t *params;
    uint32_t params_len;
} ApduInstanceParam;

/* Encryption & Ranging keys */
typedef struct {
    const uint8_t *pubkey; /* public key or key material */
    uint32_t pubkey_len;
    const uint8_t *meta;   /* metadata (alg id, usage) */
    uint32_t meta_len;
} ApduCreateKeyParam;

typedef struct {
    const uint8_t *key_ids; /* array of key ids to delete */
    uint32_t key_ids_len;
} ApduDeleteKeyParam;

/* SIGN */
typedef struct {
    const uint8_t *hash; /* digest to sign */
    uint32_t hash_len;
    uint8_t alg; /* algorithm id (e.g., ECDSA P-256) */
} ApduSignParam;

/* PRIVATE/CONFIDENTIAL DATA get/set */
typedef struct {
    uint16_t tag;
    uint32_t offset;
    uint32_t length;
} ApduGetPrivateDataParam;

typedef struct {
    uint16_t tag;
    const uint8_t *data;
    uint32_t data_len;
} ApduSetPrivateDataParam;

/* WRITE/READ BUFFER (with offsets) */
typedef struct {
    uint32_t offset;
    const uint8_t *data;
    uint32_t data_len;
} ApduWriteBufferParam;

typedef struct {
    uint32_t offset;
    uint32_t length;
} ApduReadBufferParam;

/* AUTH0 / AUTH1 / PRESENCE0 / PRESENCE1 (opaque authentication payloads or nonces) */
typedef struct {
    const uint8_t *auth_payload;
    uint32_t auth_payload_len;
} ApduAuthParam;

/* EXCHANGE (opaque payload, used for arbitrary data exchange) */
typedef struct {
    const uint8_t *payload;
    uint32_t payload_len;
} ApduExchangeParam;

/* GET_NOTIFICATION (notification id or sequence) */
typedef struct {
    uint8_t notification_id;
    uint32_t le;
} ApduGetNotificationParam;

/* Serialized wire buffer */
typedef struct {
    uint8_t *buf;
    uint32_t len;
} ApduWireBuffer;

typedef struct {
    const uint8_t *aid;
    uint32_t aid_len;
    uint8_t p1;
    uint8_t p2;
} ApduSelectParam;

typedef struct {
    const uint8_t *spake_blob;
    uint32_t spake_blob_len;
} ApduSpakeParam;

typedef struct {
    const uint8_t *tlv;
    uint32_t tlv_len;
    uint32_t offset;
} ApduWriteDataParam;

typedef struct {
    uint16_t tag;
    uint8_t p1;
    uint8_t p2;
    uint32_t le;
} ApduGetDataParam;

typedef struct {
    uint32_t le;
} ApduGetResponseParam;

typedef struct {
    uint8_t opcode;
    const uint8_t *payload;
    uint32_t payload_len;
} ApduOpControlFlowParam;

typedef struct {
    uint8_t endpoint_id;
    const uint8_t *params;
    uint32_t params_len;
} ApduEndpointParam;

typedef struct {
    uint8_t instance_id;
    const uint8_t *params;
    uint32_t params_len;
} ApduInstanceParam;

typedef struct {
    const uint8_t *pubkey;
    uint32_t pubkey_len;
    const uint8_t *meta;
    uint32_t meta_len;
} ApduCreateKeyParam;

typedef struct {
    const uint8_t *key_ids;
    uint32_t key_ids_len;
} ApduDeleteKeyParam;

typedef struct {
    uint8_t alg;
    const uint8_t *hash;
    uint32_t hash_len;
} ApduSignParam;

typedef struct {
    uint16_t tag;
    uint32_t offset;
    uint32_t length;
} ApduGetPrivateDataParam;

typedef struct {
    uint16_t tag;
    const uint8_t *data;
    uint32_t data_len;
} ApduSetPrivateDataParam;

typedef struct {
    uint32_t offset;
    const uint8_t *data;
    uint32_t data_len;
} ApduWriteBufferParam;

typedef struct {
    uint32_t offset;
    uint32_t length;
} ApduReadBufferParam;

typedef struct {
    const uint8_t *auth_payload;
    uint32_t auth_payload_len;
} ApduAuthParam;

typedef struct {
    const uint8_t *payload;
    uint32_t payload_len;
} ApduExchangeParam;

typedef struct {
    uint8_t notification_id;
    uint16_t le;
} ApduGetNotificationParam;

/* Generic param (fallback) */
typedef struct {
    const uint8_t *data;
    uint32_t data_len;
    uint32_t le;
    uint8_t cla_override; /* 0xFF means no override */
} ApduGenericParam;

/* New types used by SeInCarServices.c */
typedef struct {
    uint8_t key_id;
} ApduKeyIdParam;

typedef struct {
    uint8_t key_id;
    uint8_t flags;
    const uint8_t *meta;
    uint32_t meta_len;
} ApduKeyProvisionParam;

typedef struct {
    const uint8_t *data;
    uint32_t len;
} ApduBlobParam;

/* Helper: default AID (CCC framework) */
static const uint8_t CCC_FRAMEWORK_AID[] = {
    0xA0,0x00,0x00,0x08,0x09,0x43,0x43,0x43,0x44,0x4B,0x46,0x76,0x31
};
static const uint32_t CCC_FRAMEWORK_AID_LEN = sizeof(CCC_FRAMEWORK_AID);

/* Number of commands (must match ApduCommandId enum in ApduUtilities.h) */
#define APDU_CMD_COUNT_MIN 32

#endif /* APDU_TYPES_H */
