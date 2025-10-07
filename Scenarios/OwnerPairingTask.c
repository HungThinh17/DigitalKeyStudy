#include "OwnerPairingTask.h"
#include "NfcService.h"
#include "BleService.h"
#include "SeService.h"
#include "apduCommand.h"
#include "stuffDummy.h"
#include <stdio.h>
#include <string.h>

static bool run_pairing_sequence(void);

bool OwnerPairing_Execute(void)
{
    printf("\n=== [Owner Pairing Task Start] ===\n");

    // ---------------------------------------------------------------------
    // 1. Initialize Secure Element
    // ---------------------------------------------------------------------
    SeBusConfig seCfg = {
        .bus = SE_BUS_I2C,
        .cfg.i2c.address = 0x48,
        .cfg.i2c.frequency_hz = 400000
    };

    if (!SEService_Init(&seCfg) || !SEService_IsReady()) {
        printf("[OwnerPairing] ERROR: SE not ready\n");
        return false;
    }

    // ---------------------------------------------------------------------
    // 2. Attempt NFC-based pairing first
    // ---------------------------------------------------------------------
    bool pairingOk = false;
    bool nfcOk = false;

    printf("[OwnerPairing] Trying NFC link first...\n");

    if (NFCService_Init(NFC_TECH_A)) {
        NfcId2 id = {.id = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}};
        nfcOk = NFCService_Connect(&id);

        if (nfcOk) {
            printf("[OwnerPairing] NFC link established.\n");
            pairingOk = run_pairing_sequence();
            NFCService_Disconnect();
        } else {
            printf("[OwnerPairing] NFC link failed.\n");
        }
    } else {
        printf("[OwnerPairing] NFC initialization failed.\n");
    }

    // ---------------------------------------------------------------------
    // 3. Fallback to BLE if NFC failed
    // ---------------------------------------------------------------------
    if (!pairingOk) {
        printf("[OwnerPairing] Falling back to BLE...\n");
        if (BleService_Init(BLE_ROLE_VEHICLE_PERIPHERAL)) {
            BleService_Start();
            BleService_Connect();
            pairingOk = run_pairing_sequence();
            BleService_Disconnect();
        } else {
            printf("[OwnerPairing] BLE initialization failed.\n");
        }
    }

    // ---------------------------------------------------------------------
    // 4. Report final result
    // ---------------------------------------------------------------------
    if (pairingOk)
        printf("[OwnerPairing] ✅ Owner Pairing completed successfully\n");
    else
        printf("[OwnerPairing] ❌ Owner Pairing failed via all transports\n");

    printf("=== [Owner Pairing Task End] ===\n\n");
    return pairingOk;
}

// ============================================================================
// Internal pairing sequence (transport-agnostic)
// ============================================================================
static bool run_pairing_sequence(void)
{
    ApduResponse resp;
    memset(&resp, 0, sizeof(resp));

    printf("[OwnerPairing] Starting SPAKE2+ Authentication...\n");

    DummyKeys keys;
    StuffDummy_GetSpake2Keys(&keys);

    if (!apduCommand_SPAKE2_Request(keys.devicePub, sizeof(keys.devicePub), &resp)) {
        printf("[OwnerPairing] SPAKE2 REQUEST failed\n");
        return false;
    }

    if (!apduCommand_SPAKE2_Verify(keys.deviceProof, sizeof(keys.deviceProof), &resp)) {
        printf("[OwnerPairing] SPAKE2 VERIFY failed\n");
        return false;
    }

    printf("[OwnerPairing] SPAKE2+ authentication complete.\n");

    DummyOwnerProvision prov;
    StuffDummy_GetOwnerProvision(&prov);

    printf("[OwnerPairing] Sending Owner Provisioning Data...\n");
    if (!apduCommand_SetupInstance(prov.provData, prov.dataLen, &resp)) {
        printf("[OwnerPairing] SetupInstance failed\n");
        return false;
    }

    if (resp.status == 0x9000) {
        printf("[OwnerPairing] Owner Provisioning accepted.\n");
        StuffDummy_StoreVehicleOwner(prov.ownerId);
        return true;
    }

    printf("[OwnerPairing] Provisioning failed, SW=0x%04X\n", resp.status);
    return false;
}
