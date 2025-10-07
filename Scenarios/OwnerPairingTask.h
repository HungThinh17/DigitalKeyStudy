/**
 * @file OwnerPairingTask.h
 * @brief Task coordinator for Owner Pairing scenario (Vehicle side).
 *
 * Uses:
 *  - NFCService or BleService for transport
 *  - SeService for Secure Element access
 *  - apduCommand for SE command execution
 *  - stuffDummy for external mock data (OEM server, device input)
 *
 * Reference: CCC Digital Key v4.0.0
 *   Section 13 (Owner Pairing)
 *   Section 15 (Applet Commands)
 *   Section 14 (Authentication)
 */

#ifndef OWNER_PAIRING_TASK_H
#define OWNER_PAIRING_TASK_H

#include <stdbool.h>

/**
 * @brief Execute the Owner Pairing flow.
 *
 * Steps:
 *  1. Initialize SE and communication services.
 *  2. Establish link (BLE or NFC).
 *  3. Run SPAKE2+ authentication via APDU.
 *  4. Exchange and verify owner provisioning data.
 *  5. Confirm successful owner registration.
 *
 * @return true if pairing succeeded.
 */
bool OwnerPairing_Execute(void);

#endif /* OWNER_PAIRING_TASK_H */
