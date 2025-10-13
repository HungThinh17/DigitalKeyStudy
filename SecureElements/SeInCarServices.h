#ifndef SE_IN_CAR_SERVICES_H
#define SE_IN_CAR_SERVICES_H

#include <stdint.h>
#include "ApduTypes.h"
#include "ApduUtilities.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    SE_ERR_UNKNOWN
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
/* Lifecycle APIs                                            */
/* --------------------------------------------------------- */
SeStatus_t Se_Init(void);
void       Se_Deinit(void);

SeStatus_t Se_PowerOn(void);
SeStatus_t Se_PowerOff(void);

SeStatus_t Se_Reset(void);

/* --------------------------------------------------------- */
/* Access control + status                                   */
/* --------------------------------------------------------- */
SeStatus_t Se_BeginAccess(void);
SeStatus_t Se_EndAccess(void);

SeStatus_t Se_IsReady(void);
SeState_t  Se_GetState(void);

/* --------------------------------------------------------- */
/* APDU send core                                            */
/* --------------------------------------------------------- */
SeStatus_t Se_SendApduSync(ApduCommandId cmd, const void *param,
                           ApduResponse *resp, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* SE_IN_CAR_SERVICES_H */
