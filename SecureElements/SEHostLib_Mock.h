#ifndef SE_HOST_LIB_H
#define SE_HOST_LIB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SEHOST_OK = 0,
    SEHOST_ERR_COMM = -1,
    SEHOST_ERR_HW = -2,
    SEHOST_ERR_TIMEOUT = -3,
    SEHOST_ERR_PARAM = -4
} SEHostStatus;

/* Initialization and power control */
SEHostStatus SEHost_Init(void);
SEHostStatus SEHost_Deinit(void);
SEHostStatus SEHost_PowerOn(void);
SEHostStatus SEHost_PowerOff(void);
SEHostStatus SEHost_Reset(void);

/* Transmit an APDU (tx) and receive response (rx).
   Caller must free(*out_rx) after use. */
SEHostStatus SEHost_Transmit(const uint8_t *tx,
                             uint32_t tx_len,
                             uint8_t **out_rx,
                             uint32_t *out_rx_len,
                             uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* SE_HOST_LIB_H */
