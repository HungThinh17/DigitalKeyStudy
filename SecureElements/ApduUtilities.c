#include "ApduUtilities.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Descriptor defaults (CLA/INS/P1/P2) aligned with CCC spec sections */
typedef struct {
    ApduCommandId id;
    uint8_t cla;
    uint8_t ins;
    uint8_t p1;
    uint8_t p2;
    const char *desc;
} CmdDesc;

/* Command table: keep existing commands first, then extended ones */
static const CmdDesc cmd_table[] = {
    { APDU_CMD_GENERIC,              0x00, 0x00, 0x00, 0x00, "APDU_CMD_GENERIC" },
    { APDU_CMD_SELECT,               0x00, 0xA4, 0x04, 0x00, "SELECT" },
    { APDU_CMD_SPAKE2_REQUEST,       0x00, 0x30, 0x00, 0x00, "SPAKE2+ REQUEST" },
    { APDU_CMD_SPAKE2_VERIFY,        0x00, 0x32, 0x00, 0x00, "SPAKE2+ VERIFY" },
    { APDU_CMD_WRITE_DATA,           0x80, 0xD4, 0x00, 0x00, "WRITE DATA" },
    { APDU_CMD_GET_DATA,             0x00, 0xCA, 0x00, 0x00, "GET DATA" },
    { APDU_CMD_GET_RESPONSE,         0x00, 0xC0, 0x00, 0x00, "GET RESPONSE" },
    { APDU_CMD_OP_CONTROL_FLOW,      0x80, 0x3C, 0x00, 0x00, "OP CONTROL FLOW" },
    { APDU_CMD_CREATE_ENDPOINT,      0x80, 0xE0, 0x00, 0x00, "CREATE ENDPOINT" },
    { APDU_CMD_SETUP_ENDPOINT,       0x80, 0xE2, 0x00, 0x00, "SETUP ENDPOINT" },
    { APDU_CMD_AUTHORIZE_ENDPOINT,   0x80, 0xE4, 0x00, 0x00, "AUTHORIZE ENDPOINT" },
    { APDU_CMD_SETUP_INSTANCE,       0x80, 0xE6, 0x00, 0x00, "SETUP INSTANCE" },
    { APDU_CMD_TERMINATE_ENDPOINT,   0x80, 0xE8, 0x00, 0x00, "TERMINATE ENDPOINT" },
    { APDU_CMD_DELETE_ENDPOINT,      0x80, 0xEA, 0x00, 0x00, "DELETE ENDPOINT" },
    { APDU_CMD_CONVERT_ENDPOINT,     0x80, 0xEC, 0x00, 0x00, "CONVERT ENDPOINT" },
    { APDU_CMD_CREATE_ENCRYPTION_KEY,0x80, 0xF0, 0x00, 0x00, "CREATE ENCRYPTION KEY" },
    { APDU_CMD_CREATE_RANGING_KEY,   0x80, 0xF2, 0x00, 0x00, "CREATE RANGING KEY" },
    { APDU_CMD_DELETE_RANGING_KEYS,  0x80, 0xF4, 0x00, 0x00, "DELETE RANGING KEYS" },
    { APDU_CMD_SIGN,                 0x80, 0x2A, 0x00, 0x00, "SIGN" },
    { APDU_CMD_GET_PRIVATE_DATA,     0x80, 0xCA, 0x00, 0x00, "GET PRIVATE DATA" },
    { APDU_CMD_SET_PRIVATE_DATA,     0x80, 0xDA, 0x00, 0x00, "SET PRIVATE DATA" },
    { APDU_CMD_SET_CONFIDENTIAL_DATA,0x80, 0xDB, 0x00, 0x00, "SET CONFIDENTIAL DATA" },
    { APDU_CMD_WRITE_BUFFER,         0x80, 0xE1, 0x00, 0x00, "WRITE BUFFER" },
    { APDU_CMD_READ_BUFFER,          0x80, 0xE3, 0x00, 0x00, "READ BUFFER" },
    { APDU_CMD_AUTH0,                0x00, 0xF5, 0x00, 0x00, "AUTH0" },
    { APDU_CMD_AUTH1,                0x00, 0xF6, 0x00, 0x00, "AUTH1" },
    { APDU_CMD_PRESENCE0,            0x00, 0xF7, 0x00, 0x00, "PRESENCE0" },
    { APDU_CMD_PRESENCE1,            0x00, 0xF8, 0x00, 0x00, "PRESENCE1" },
    { APDU_CMD_EXCHANGE,             0x80, 0xFE, 0x00, 0x00, "EXCHANGE" },
    { APDU_CMD_GET_NOTIFICATION,     0x00, 0xB2, 0x00, 0x00, "GET NOTIFICATION" },

    /* CCC Digital Key v4.0 Secure Element service extensions */
    { APDU_CMD_GET_PROVISION_STATUS, 0x80, 0x10, 0x00, 0x00, "GET PROVISION STATUS" },
    { APDU_CMD_GET_PROVISION_INFO,   0x80, 0x11, 0x00, 0x00, "GET PROVISION INFO" },
    { APDU_CMD_GET_PAIRING_STATUS,   0x80, 0x20, 0x00, 0x00, "GET PAIRING STATUS" },
    { APDU_CMD_KEY_GET_INFO,         0x80, 0x21, 0x00, 0x00, "KEY GET INFO" },
    { APDU_CMD_KEY_PROVISION,        0x80, 0x22, 0x00, 0x00, "KEY PROVISION" },
    { APDU_CMD_KEY_REVOKE,           0x80, 0x23, 0x00, 0x00, "KEY REVOKE" },
    { APDU_CMD_KEY_SHARE,            0x80, 0x24, 0x00, 0x00, "KEY SHARE" },
    { APDU_CMD_AUTH_CHALLENGE,       0x80, 0x30, 0x00, 0x00, "AUTH CHALLENGE" },
    { APDU_CMD_AUTH_VERIFY,          0x80, 0x31, 0x00, 0x00, "AUTH VERIFY" },
};

/* helpers */
static uint8_t *memdup_u8(const uint8_t *p, uint32_t n) {
    if (!p || n == 0) return NULL;
    uint8_t *b = (uint8_t*)malloc(n);
    if (!b) return NULL;
    memcpy(b, p, n);
    return b;
}

/* build command - now uses typed param structs via void* casting */
int apdu_build_command(ApduCommandId cmd, const void *param, ApduFrame *out) {
    if (!out) return -1;
    if (cmd < 0 || cmd >= APDU_CMD_COUNT) return -1;

    const CmdDesc *d = &cmd_table[cmd];

    /* zero out frame */
    out->cla = d->cla;
    out->ins = d->ins;
    out->p1  = d->p1;
    out->p2  = d->p2;
    out->data = NULL;
    out->lc = 0;
    out->le = 0;

    /* switch and cast */
    switch (cmd) {
        case APDU_CMD_SELECT: {
            const ApduSelectParam *p = (const ApduSelectParam*)param;
            if (p && p->aid && p->aid_len > 0) {
                out->data = memdup_u8(p->aid, p->aid_len);
                out->lc = p->aid_len;
                if (p->p1) out->p1 = p->p1;
                out->p2 = p->p2;
            } else {
                out->data = memdup_u8((const uint8_t*)CCC_FRAMEWORK_AID, CCC_FRAMEWORK_AID_LEN);
                out->lc = CCC_FRAMEWORK_AID_LEN;
            }
            break;
        }

        case APDU_CMD_SPAKE2_REQUEST:
        case APDU_CMD_SPAKE2_VERIFY: {
            const ApduSpakeParam *p = (const ApduSpakeParam*)param;
            if (!p || !p->spake_blob || p->spake_blob_len == 0) return -1;
            out->data = memdup_u8(p->spake_blob, p->spake_blob_len);
            out->lc = p->spake_blob_len;
            break;
        }

        case APDU_CMD_WRITE_DATA: {
            const ApduWriteDataParam *p = (const ApduWriteDataParam*)param;
            if (!p || !p->tlv || p->tlv_len == 0) return -1;
            /* If offset is used for chunking, caller is expected to encode TLV chunk; here we copy the supplied TLV */
            out->data = memdup_u8(p->tlv, p->tlv_len);
            out->lc = p->tlv_len;
            break;
        }

        case APDU_CMD_GET_DATA: {
            const ApduGetDataParam *p = (const ApduGetDataParam*)param;
            /* encode request as tag (2 bytes big-endian) + optional le (if provided) */
            if (!p) return -1;
            uint8_t tmp[4];
            tmp[0] = (uint8_t)((p->tag >> 8) & 0xFF);
            tmp[1] = (uint8_t)(p->tag & 0xFF);
            /* optionally include p1/p2 in header override */
            out->p1 = p->p1;
            out->p2 = p->p2;
            out->data = memdup_u8(tmp, 2);
            out->lc = 2;
            out->le = p->le;
            break;
        }

        case APDU_CMD_GET_RESPONSE: {
            const ApduGetResponseParam *p = (const ApduGetResponseParam*)param;
            if (!p) {
                /* default: Le = 256 meaning extended expected; keep 0 (not present) */
                out->le = 0;
            } else {
                out->le = p->le;
            }
            break;
        }

        case APDU_CMD_OP_CONTROL_FLOW: {
            const ApduOpControlFlowParam *p = (const ApduOpControlFlowParam*)param;
            if (!p) return -1;
            uint8_t hdr[1];
            hdr[0] = p->opcode;
            out->data = memdup_u8(hdr, 1);
            out->lc = 1;
            if (p->payload && p->payload_len) {
                /* append payload */
                uint8_t *conc = (uint8_t*)malloc(out->lc + p->payload_len);
                if (!conc) return -1;
                memcpy(conc, out->data, out->lc);
                memcpy(conc + out->lc, p->payload, p->payload_len);
                free(out->data);
                out->data = conc;
                out->lc += p->payload_len;
            }
            break;
        }

        case APDU_CMD_CREATE_ENDPOINT:
        case APDU_CMD_SETUP_ENDPOINT:
        case APDU_CMD_AUTHORIZE_ENDPOINT:
        case APDU_CMD_TERMINATE_ENDPOINT:
        case APDU_CMD_DELETE_ENDPOINT:
        case APDU_CMD_CONVERT_ENDPOINT: {
            const ApduEndpointParam *p = (const ApduEndpointParam*)param;
            if (!p) return -1;
            /* encode: endpoint id (1 byte) + params */
            uint32_t l = 1 + (p->params_len);
            uint8_t *buf = (uint8_t*)malloc(l);
            if (!buf) return -1;
            buf[0] = p->endpoint_id;
            if (p->params && p->params_len) memcpy(&buf[1], p->params, p->params_len);
            out->data = buf;
            out->lc = l;
            break;
        }

        case APDU_CMD_SETUP_INSTANCE: {
            const ApduInstanceParam *p = (const ApduInstanceParam*)param;
            if (!p) return -1;
            uint32_t l = 1 + p->params_len;
            uint8_t *buf = (uint8_t*)malloc(l);
            if (!buf) return -1;
            buf[0] = p->instance_id;
            if (p->params && p->params_len) memcpy(&buf[1], p->params, p->params_len);
            out->data = buf;
            out->lc = l;
            break;
        }

        case APDU_CMD_CREATE_ENCRYPTION_KEY:
        case APDU_CMD_CREATE_RANGING_KEY: {
            const ApduCreateKeyParam *p = (const ApduCreateKeyParam*)param;
            if (!p || (!p->pubkey && p->pubkey_len==0)) return -1;
            /* simple concat: [pubkey_len(2)] [pubkey] [meta_len(2)] [meta] */
            uint32_t l = 2 + p->pubkey_len + 2 + p->meta_len;
            uint8_t *buf = (uint8_t*)malloc(l);
            if (!buf) return -1;
            buf[0] = (uint8_t)((p->pubkey_len >> 8) & 0xFF);
            buf[1] = (uint8_t)(p->pubkey_len & 0xFF);
            memcpy(&buf[2], p->pubkey, p->pubkey_len);
            buf[2 + p->pubkey_len] = (uint8_t)((p->meta_len >> 8) & 0xFF);
            buf[3 + p->pubkey_len] = (uint8_t)(p->meta_len & 0xFF);
            if (p->meta && p->meta_len) memcpy(&buf[4 + p->pubkey_len], p->meta, p->meta_len);
            out->data = buf;
            out->lc = l;
            break;
        }

        case APDU_CMD_DELETE_RANGING_KEYS: {
            const ApduDeleteKeyParam *p = (const ApduDeleteKeyParam*)param;
            if (!p || p->key_ids_len == 0) return -1;
            out->data = memdup_u8(p->key_ids, p->key_ids_len);
            out->lc = p->key_ids_len;
            break;
        }

        case APDU_CMD_SIGN: {
            const ApduSignParam *p = (const ApduSignParam*)param;
            if (!p || !p->hash || p->hash_len==0) return -1;
            /* encode: alg(1) | hash_len(2) | hash */
            uint32_t l = 1 + 2 + p->hash_len;
            uint8_t *buf = (uint8_t*)malloc(l);
            if (!buf) return -1;
            buf[0] = p->alg;
            buf[1] = (uint8_t)((p->hash_len >> 8) & 0xFF);
            buf[2] = (uint8_t)(p->hash_len & 0xFF);
            memcpy(&buf[3], p->hash, p->hash_len);
            out->data = buf;
            out->lc = l;
            break;
        }

        case APDU_CMD_GET_PRIVATE_DATA: {
            const ApduGetPrivateDataParam *p = (const ApduGetPrivateDataParam*)param;
            if (!p) return -1;
            uint8_t tmp[8];
            tmp[0] = (uint8_t)((p->tag >> 8) & 0xFF);
            tmp[1] = (uint8_t)(p->tag & 0xFF);
            tmp[2] = (uint8_t)((p->offset >> 24) & 0xFF);
            tmp[3] = (uint8_t)((p->offset >> 16) & 0xFF);
            tmp[4] = (uint8_t)((p->offset >> 8) & 0xFF);
            tmp[5] = (uint8_t)(p->offset & 0xFF);
            tmp[6] = (uint8_t)((p->length >> 8) & 0xFF);
            tmp[7] = (uint8_t)(p->length & 0xFF);
            out->data = memdup_u8(tmp, 8);
            out->lc = 8;
            break;
        }

        case APDU_CMD_SET_PRIVATE_DATA:
        case APDU_CMD_SET_CONFIDENTIAL_DATA: {
            const ApduSetPrivateDataParam *p = (const ApduSetPrivateDataParam*)param;
            if (!p || !p->data || p->data_len==0) return -1;
            uint8_t hdr[2];
            hdr[0] = (uint8_t)((p->tag >> 8) & 0xFF);
            hdr[1] = (uint8_t)(p->tag & 0xFF);
            uint8_t *buf = (uint8_t*)malloc(2 + p->data_len);
            if (!buf) return -1;
            memcpy(buf, hdr, 2);
            memcpy(buf + 2, p->data, p->data_len);
            out->data = buf;
            out->lc = 2 + p->data_len;
            break;
        }

        case APDU_CMD_WRITE_BUFFER: {
            const ApduWriteBufferParam *p = (const ApduWriteBufferParam*)param;
            if (!p || !p->data || p->data_len==0) return -1;
            /* encode offset(4) + data */
            uint32_t l = 4 + p->data_len;
            uint8_t *buf = (uint8_t*)malloc(l);
            if (!buf) return -1;
            buf[0] = (uint8_t)((p->offset >> 24) & 0xFF);
            buf[1] = (uint8_t)((p->offset >> 16) & 0xFF);
            buf[2] = (uint8_t)((p->offset >> 8) & 0xFF);
            buf[3] = (uint8_t)(p->offset & 0xFF);
            memcpy(&buf[4], p->data, p->data_len);
            out->data = buf;
            out->lc = l;
            break;
        }

        case APDU_CMD_READ_BUFFER: {
            const ApduReadBufferParam *p = (const ApduReadBufferParam*)param;
            if (!p) return -1;
            uint8_t tmp[8];
            tmp[0] = (uint8_t)((p->offset >> 24) & 0xFF);
            tmp[1] = (uint8_t)((p->offset >> 16) & 0xFF);
            tmp[2] = (uint8_t)((p->offset >> 8) & 0xFF);
            tmp[3] = (uint8_t)(p->offset & 0xFF);
            tmp[4] = (uint8_t)((p->length >> 24) & 0xFF);
            tmp[5] = (uint8_t)((p->length >> 16) & 0xFF);
            tmp[6] = (uint8_t)((p->length >> 8) & 0xFF);
            tmp[7] = (uint8_t)(p->length & 0xFF);
            out->data = memdup_u8(tmp, 8);
            out->lc = 8;
            break;
        }

        case APDU_CMD_AUTH0:
        case APDU_CMD_AUTH1:
        case APDU_CMD_PRESENCE0:
        case APDU_CMD_PRESENCE1: {
            const ApduAuthParam *p = (const ApduAuthParam*)param;
            if (!p || !p->auth_payload || p->auth_payload_len==0) return -1;
            out->data = memdup_u8(p->auth_payload, p->auth_payload_len);
            out->lc = p->auth_payload_len;
            break;
        }

        case APDU_CMD_EXCHANGE: {
            const ApduExchangeParam *p = (const ApduExchangeParam*)param;
            if (!p || !p->payload || p->payload_len==0) return -1;
            out->data = memdup_u8(p->payload, p->payload_len);
            out->lc = p->payload_len;
            break;
        }

        case APDU_CMD_GET_NOTIFICATION: {
            const ApduGetNotificationParam *p = (const ApduGetNotificationParam*)param;
            if (!p) return -1;
            uint8_t tmp[3];
            tmp[0] = p->notification_id;
            tmp[1] = (uint8_t)((p->le >> 8) & 0xFF);
            tmp[2] = (uint8_t)(p->le & 0xFF);
            out->data = memdup_u8(tmp, 3);
            out->lc = 3;
            break;
        }

        /* -------------------------- New CCC commands -------------------------- */
        case APDU_CMD_GET_PROVISION_STATUS:
        case APDU_CMD_GET_PROVISION_INFO:
        case APDU_CMD_GET_PAIRING_STATUS:
        case APDU_CMD_AUTH_CHALLENGE: {
            /* these commands have no body; param ignored */
            break;
        }

        case APDU_CMD_KEY_GET_INFO:
        case APDU_CMD_KEY_REVOKE: {
            const ApduKeyIdParam *p = (const ApduKeyIdParam*)param;
            if (!p) return -1;
            uint8_t b = p->key_id;
            out->data = memdup_u8(&b, 1);
            out->lc = 1;
            break;
        }

        case APDU_CMD_KEY_PROVISION: {
            const ApduKeyProvisionParam *p = (const ApduKeyProvisionParam*)param;
            if (!p) return -1;
            /* encode: key_id (1) | flags (1) | meta_len(2) | meta */
            uint32_t l = 1 + 1 + 2 + (p->meta_len);
            uint8_t *buf = (uint8_t*)malloc(l);
            if (!buf) return -1;
            uint32_t idx = 0;
            buf[idx++] = p->key_id;
            buf[idx++] = p->flags;
            buf[idx++] = (uint8_t)((p->meta_len >> 8) & 0xFF);
            buf[idx++] = (uint8_t)(p->meta_len & 0xFF);
            if (p->meta && p->meta_len) memcpy(&buf[idx], p->meta, p->meta_len);
            out->data = buf;
            out->lc = l;
            break;
        }

        case APDU_CMD_KEY_SHARE:
        case APDU_CMD_AUTH_VERIFY: {
            const ApduBlobParam *p = (const ApduBlobParam*)param;
            if (!p || !p->data || p->len==0) return -1;
            out->data = memdup_u8(p->data, p->len);
            out->lc = p->len;
            break;
        }

        case APDU_CMD_GENERIC:
        default:
            /* fallback: if param is ApduGenericParam style, accept it */
            if (param) {
                const ApduGenericParam *g = (const ApduGenericParam*)param;
                if (g->data && g->data_len > 0) {
                    out->data = memdup_u8(g->data, g->data_len);
                    out->lc = g->data_len;
                }
                if (g->le > 0) out->le = g->le;
                if (g->cla_override != 0xFF) out->cla = g->cla_override;
            } else {
                /* nothing to do - command with no data */
            }
            break;
    }

    return 0;
}

/* Serialize APDU into wire buffer (ISO-7816 support: short & extended) */
int apdu_serialize_frame(const ApduFrame *frame, ApduWireBuffer *wire) {
    if (!frame || !wire) return -1;
    uint32_t lc = frame->lc;
    uint32_t le = frame->le;
    int extended = 0;
    uint32_t size = 4; /* header */

    if (lc == 0) {
        if (le == 0) {
            /* 4 bytes only */
        } else if (le <= 0xFF) {
            size += 1;
        } else {
            extended = 1;
            size += 3; /* 0x00 + Le(2) */
        }
    } else {
        if (lc <= 0xFF) {
            size += 1 + lc;
            if (le > 0) {
                if (le <= 0xFF) size += 1; else { size += 3; extended = 1; }
            }
        } else {
            extended = 1;
            size += 3 + lc; /* 0x00 + Lc(2) + data */
            if (le > 0) size += 2; /* Le(2) */
        }
    }

    uint8_t *buf = (uint8_t*)malloc(size);
    if (!buf) return -1;
    uint32_t idx = 0;
    buf[idx++] = frame->cla;
    buf[idx++] = frame->ins;
    buf[idx++] = frame->p1;
    buf[idx++] = frame->p2;

    if (lc == 0) {
        if (le == 0) {
            /* nothing more */
        } else if (!extended) {
            buf[idx++] = (uint8_t)(le & 0xFF);
        } else {
            buf[idx++] = 0x00;
            buf[idx++] = (uint8_t)((le >> 8) & 0xFF);
            buf[idx++] = (uint8_t)(le & 0xFF);
        }
    } else {
        if (lc <= 0xFF) {
            buf[idx++] = (uint8_t)(lc & 0xFF);
            if (frame->data && lc) {
                memcpy(&buf[idx], frame->data, lc);
                idx += lc;
            }
            if (le > 0) {
                if (le <= 0xFF) buf[idx++] = (uint8_t)(le & 0xFF);
                else {
                    buf[idx++] = 0x00;
                    buf[idx++] = (uint8_t)((le >> 8) & 0xFF);
                    buf[idx++] = (uint8_t)(le & 0xFF);
                }
            }
        } else {
            buf[idx++] = 0x00;
            buf[idx++] = (uint8_t)((lc >> 8) & 0xFF);
            buf[idx++] = (uint8_t)(lc & 0xFF);
            if (frame->data && lc) {
                memcpy(&buf[idx], frame->data, lc);
                idx += lc;
            }
            if (le > 0) {
                buf[idx++] = (uint8_t)((le >> 8) & 0xFF);
                buf[idx++] = (uint8_t)(le & 0xFF);
            }
        }
    }

    wire->buf = buf;
    wire->len = idx;
    return 0;
}

void apdu_free_wirebuffer(ApduWireBuffer *wire) {
    if (!wire) return;
    if (wire->buf) free(wire->buf);
    wire->buf = NULL;
    wire->len = 0;
}

/* Response parse */
int apdu_parse_response(const uint8_t *raw, uint32_t len, ApduResponse *resp) {
    if (!raw || !resp || len < 2) return -1;
    uint32_t data_len = (len > 2) ? (len - 2) : 0;
    resp->data = NULL;
    resp->len = 0;
    if (data_len > 0) {
        resp->data = memdup_u8(raw, data_len);
        if (!resp->data) return -1;
        resp->len = data_len;
    }
    resp->sw1 = raw[len - 2];
    resp->sw2 = raw[len - 1];
    return 0;
}

void apdu_free_response(ApduResponse *resp) {
    if (!resp) return;
    if (resp->data) free(resp->data);
    resp->data = NULL;
    resp->len = 0;
    resp->sw1 = resp->sw2 = 0;
}

/* Free ApduFrame internal data */
void apdu_free_frame(ApduFrame *frame) {
    if (!frame) return;
    if (frame->data) free(frame->data);
    frame->data = NULL;
    frame->lc = 0;
    frame->le = 0;
}

/* Debug print */
void apdu_dump_wire(const uint8_t *buf, uint32_t len) {
    if (!buf || len==0) { printf("[apdu_dump_wire] empty\n"); return; }
    for (uint32_t i=0;i<len;i++) {
        printf("%02X", buf[i]);
        if (i+1<len) printf(" ");
    }
    printf("\n");
}

/* DK_APDU_RQ helpers (simple wrapper per earlier design) */
int apdu_is_allowed_cla_for_dk_apdu(uint8_t cla) {
    return (cla == 0x00 || cla == 0x80 || cla == 0x84) ? 1 : 0;
}

int apdu_wrap_dk_apdu_rq(const ApduWireBuffer *apdu_wire, ApduWireBuffer *out_wrapped) {
    if (!apdu_wire || !out_wrapped) return -1;
    if (!apdu_wire->buf || apdu_wire->len==0) return -1;
    if (!apdu_is_allowed_cla_for_dk_apdu(apdu_wire->buf[0])) return -2;

    uint32_t total = 1 + 2 + apdu_wire->len;
    uint8_t *b = (uint8_t*)malloc(total);
    if (!b) return -1;
    uint32_t idx = 0;
    b[idx++] = 0x0B; /* tag */
    b[idx++] = (uint8_t)((apdu_wire->len >> 8) & 0xFF);
    b[idx++] = (uint8_t)(apdu_wire->len & 0xFF);
    memcpy(&b[idx], apdu_wire->buf, apdu_wire->len);
    idx += apdu_wire->len;
    out_wrapped->buf = b;
    out_wrapped->len = idx;
    return 0;
}

int apdu_unwrap_dk_apdu_rq(const uint8_t *wrapped, uint32_t len, ApduWireBuffer *out_apdu_wire) {
    if (!wrapped || !out_apdu_wire || len < 3) return -1;
    if (wrapped[0] != 0x0B) return -2;
    uint32_t payload_len = ((uint32_t)wrapped[1] << 8) | (uint32_t)wrapped[2];
    if (payload_len + 3 != len) return -3;
    uint8_t *p = memdup_u8(&wrapped[3], payload_len);
    if (!p) return -1;
    out_apdu_wire->buf = p;
    out_apdu_wire->len = payload_len;
    return 0;
}
