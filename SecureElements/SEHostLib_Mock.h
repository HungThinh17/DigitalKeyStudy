#ifndef SE_HOST_LIB_H
#define SE_HOST_LIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SEHOST_UNKNOW = 0xFFFF,
    SEHOST_OK = 0x9000,
    SEHOST_ERR_SEND_FAIL = 0x7010,
    SEHOST_ERR_RECEIVE_FAIL = 0x7011
} SEHostStatus;

typedef SEHostStatus (*se_apdu_trancsceive_t)(uint8_t *tx, uint32_t tx_len,
            uint8_t *out_rx, uint32_t *out_rx_len);

/* Initialization and power control */
SEHostStatus SEHost_Init(se_apdu_trancsceive_t pTransceive);
SEHostStatus SEHost_Close(void);
SEHostStatus SEHost_Reset(void);

/* Transmit an APDU (tx) and receive response (rx).
   Caller must free(*out_rx) after use. */
SEHostStatus SEHost_Tranceive(const uint8_t *tx, uint32_t tx_len,
                             uint8_t *out_rx, uint32_t *out_rx_len);

#ifdef __cplusplus
}
#endif

#endif /* SE_HOST_LIB_H */
