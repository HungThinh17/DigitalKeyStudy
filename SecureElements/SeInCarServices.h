#ifndef SE_IN_CAR_SERVICES_H
#define SE_IN_CAR_SERVICES_H

#include <stdint.h>
#include "ApduTypes.h"
#include "ApduUtilities.h"
#include "SeInCarTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------- */
/* Lifecycle + access control                                */
/* --------------------------------------------------------- */

SeStatus_t Se_Init(void);
void       Se_Deinit(void);

SeStatus_t Se_PowerOn(void);
SeStatus_t Se_PowerOff(void);
SeStatus_t Se_Reset(void);

SeStatus_t Se_BeginAccess(void);
SeStatus_t Se_EndAccess(void);

SeStatus_t Se_IsReady(void);
SeState_t  Se_GetState(void);

/* --------------------------------------------------------- */
/* Core APDU send helper                                     */
/* --------------------------------------------------------- */
SeStatus_t Se_SendApduSync(ApduCommandId cmd, const void *param,
                           ApduResponse *resp, uint32_t timeout_ms);

/* --------------------------------------------------------- */
/* Digital Key Common Services                               */
/* --------------------------------------------------------- */

SeStatus_t Se_CheckProvisioningStatus(SeProvisionInfo *status);
SeStatus_t Se_GetProvisioningInfo(SeProvisionInfo *info);

SeStatus_t Se_Pairing_SelectApplet(void);
SeStatus_t Se_Pairing_Spake2Request(const SeSpakeBlob *req, SeSpakeBlob *resp);
SeStatus_t Se_Pairing_Spake2Verify(const SeSpakeBlob *verify, SePairingResult *result);
SeStatus_t Se_Pairing_GetStatus(SePairingStatus *status);


SeStatus_t Se_Key_GetInfo(uint8_t key_id, SeKeyInfo *info);
SeStatus_t Se_Key_Provision(const SeKeyInfo *info);
SeStatus_t Se_Key_Revoke(uint8_t key_id);
SeStatus_t Se_Key_Share(const SeShareBlob *req, SeShareBlob *resp);


SeStatus_t Se_Auth_GenerateChallenge(SeChallenge *out);
SeStatus_t Se_Auth_VerifyProof(const SeProof *in, SeAuthResult *result);

#ifdef __cplusplus
}
#endif

#endif /* SE_IN_CAR_SERVICES_H */
