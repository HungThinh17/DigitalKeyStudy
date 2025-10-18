#ifndef SE_INCAR_TYPES_H
#define SE_INCAR_TYPES_H

#include <stdint.h>
#include <stdlib.h>

/* --------------------------------------------------------- */
/* Status codes                                              */
/* --------------------------------------------------------- */
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
    SE_ERR_UNKNOWN,
    SE_ERR_NOT_PROVISIONED,
    SE_ERR_NOT_PAIRED
} SeStatus_t;

/* --------------------------------------------------------- */
/* Internal service state                                    */
/* --------------------------------------------------------- */
typedef enum {
    SE_STATE_UNINIT = 0,
    SE_STATE_IDLE,
    SE_STATE_BUSY,
    SE_STATE_ERROR
} SeState_t;

/* --------------------------------------------------------- */
/* Provisioning / configuration                              */
/* --------------------------------------------------------- */
typedef struct {
    uint32_t applet_version;
    uint8_t  provisioned;        /* 1 = ready, 0 = not */
    uint8_t  ursk_present;       /* 1 = owner paired */
    uint8_t  free_owner_slots;
    uint8_t  total_key_slots;
    uint8_t  supported_curves;   /* bitmask: bit0=P-256, bit1=P-384, etc. */
    uint8_t  oem_cert_hash[32];  /* hash of OEM cert (optional) */
} SeProvisionInfo;

/* --------------------------------------------------------- */
/* Authentication / Unlock                                   */
/* --------------------------------------------------------- */
typedef struct {
    uint8_t  challenge[32];
    uint32_t len;
} SeChallenge;

typedef struct {
    uint8_t *data;
    uint32_t len;
} SeProof;

typedef struct {
    uint8_t verified; /* 1 = success */
    uint8_t error_code;
} SeAuthResult;

/* --------------------------------------------------------- */
/* Owner pairing (atomic service calls)                      */
/* --------------------------------------------------------- */
typedef struct {
    uint8_t *data;
    uint32_t len;
} SeSpakeBlob;

typedef struct {
    uint8_t paired;  /* 1 = paired, 0 = not */
    uint8_t locked;  /* 1 = locked due to error */
} SePairingStatus;

typedef struct {
    uint8_t success; /* 1 = pairing OK, 0 = fail */
    uint8_t error_code;
} SePairingResult;

/* --------------------------------------------------------- */
/* Key management / sharing                                  */
/* --------------------------------------------------------- */
typedef struct {
    uint8_t key_id;
    uint8_t valid;
    uint8_t shared;
    uint8_t revoked;
} SeKeyInfo;

typedef struct {
    uint8_t *data;
    uint32_t len;
} SeShareBlob;

#endif /* SE_INCAR_TYPES_H */
