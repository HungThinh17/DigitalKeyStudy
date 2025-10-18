
### 1. Purpose of Owner Pairing

Owner pairing is the process that *creates the very first secure relationship* between the vehicle (in-car system) and the owner’s device (smartphone).

It achieves three things:

1. **Mutual authentication:** each side proves it’s genuine using the manufacturer-provisioned credentials.
2. **Secure key establishment:** they derive a shared secret (the *URSK*, “User Root Secret Key”) through SPAKE2+, a password-authenticated key exchange.
3. **Provisioning of digital keys:** that secret gets used by the SE to derive or protect the actual *vehicle key* material stored inside the Secure Element (SE).

After this pairing, both sides possess enough cryptographic material to later authenticate, unlock, and start the car — without ever talking to the OEM’s cloud again.

---

### 2. Major Actors / Modules

| Actor                                      | Role in pairing                                                                                                                                       |
| ------------------------------------------ | ----------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Owner device (phone)**                   | Initiates pairing, drives UI, provides user authentication (PIN, biometrics). Runs CCC stack with SPAKE2+, certificate chain, etc.                    |
| **In-car MCU (Smart Access Controller)**   | Communicates with phone over NFC or BLE, routes APDUs to SE, and handles state machine logic.                                                         |
| **Secure Element (SE)**                    | Performs the cryptographic heavy lifting and secure storage: key derivation, SPAKE2+ transcript handling, and storage of root keys & pairing records. |
| **Vehicle HSM / backend (optional)**       | May pre-personalize the SE or verify attestation data. Not always online.                                                                             |
| **SEInCarService module (your C service)** | Software bridge on MCU: constructs APDUs, sends them to SE, interprets results, and coordinates SPAKE2+ flow during pairing.                          |

---

### 3. Pairing Phases (per CCC TS-101 v4.0)

Owner pairing happens typically over **NFC** first (for physical proximity and security). The phases are:

#### Phase 0 – Preparation

* The vehicle is powered, SE initialized.
* The phone enters pairing mode (user presses “Add Vehicle Key”).
* The phone retrieves OEM-issued certificates and metadata.

#### Phase 1 – Secure Channel Establishment (SPAKE2+)

This is where your `SEInCarService` dances.

**Steps inside this phase:**

1. **SELECT Applet (APDU)**
   The MCU tells SE to select its Digital Key applet.
2. **SPAKE2+ Round 1 (Request)**

   * The phone sends its SPAKE2+ public value (X) to the car.
   * The MCU receives it, wraps it into `APDU_CMD_SPAKE2_REQUEST`, and sends it to SE.
   * SE responds with its SPAKE2+ public value (Y) and optional transcript data.
3. **SPAKE2+ Round 2 (Verification)**

   * The MCU returns SE’s SPAKE2+ Y value to the phone.
   * The phone computes the shared secret, builds the verification blob, and sends it back.
   * MCU forwards that via `APDU_CMD_SPAKE2_VERIFY` to SE.
   * SE validates it, computes the same shared secret, and — if both match — marks the channel authenticated.
4. **URSK Derivation**
   SE derives the User Root Secret Key (URSK). This key never leaves the SE. It’s used to generate operational keys (e.g., BLE/NFC unlock keys).

#### Phase 2 – Key Material Provisioning

* SE generates and stores Digital Key records internally.
* MCU confirms pairing result to phone.
* The phone receives a “Digital Key Credential,” which includes encrypted data bound to SE’s public key.

#### Phase 3 – Confirmation & Record Commit

* The SE writes pairing metadata to its secure storage (owner ID, key index, timestamps).
* MCU notifies upper layers (Vehicle Access Manager, etc.) that owner pairing succeeded.

---

### 4. Message Flow Simplified

```
Phone                    Vehicle MCU                   SE (Secure Element)
-----                    ------------                  -------------------
 |  NFC connect   --->     Detects & routes  --->        Ready
 |  SPAKE2+ X      --->    apdu_build(SPAKE2_REQ)
 |                          send APDU(SPAKE2_REQ)
 |                                                      Compute SPAKE2+ Y
 |                                                      return SPAKE2_RESP
 |  <--- recv Y ---         parse response
 |  compute shared key
 |  send verify blob  --->  APDU(SPAKE2_VERIFY)
 |                                                      verify transcript
 |                                                      derive URSK
 |  <--- success ----        0x9000 OK
 |  store key info
```

---

### 5. How `SEInCarService` fits in

`SEInCarService` sits between the transport logic and the SE. It provides these responsibilities:

1. **Lifecycle management:** power, reset, access control of SE.
2. **APDU framing/parsing:** using your `ApduUtilities.c/h`.
3. **Owner pairing orchestration:** implement the sequence:

   * `SELECT` → `SPAKE2_REQUEST` → `SPAKE2_VERIFY`.
4. **Error handling & retries:** manage timeouts, SW codes (0x61xx, 0x6Axx).
5. **Secure state caching:** track pairing progress to prevent re-entry until previous session ends.
6. **Provide API to higher layers:** e.g., `Se_OwnerPairing_Perform()` returns `SE_OK` only when the SE confirms pairing success.

In the full system architecture, `SEInCarService` is invoked by the *Smart Access Manager* or *Vehicle Access Control module*, but it’s the SE that decides success or failure cryptographically.

---

### 6. Data Involved

| Data                                    | Where it lives            | Purpose                                    |
| --------------------------------------- | ------------------------- | ------------------------------------------ |
| **SPAKE2+ ephemeral keys (X, Y)**       | Phone & SE (in memory)    | Used for key exchange                      |
| **URSK**                                | SE secure storage         | Root key for all future derivations        |
| **Owner pairing record**                | SE NVM / EEPROM           | Stores binding between vehicle ID and user |
| **Certificates (OEM, Vehicle, Device)** | Used by both phone and SE | For attestation & chain validation         |
| **APDU exchanges**                      | MCU ↔ SE                  | Carry SPAKE2 data and responses            |

---

### 7. Security Summary

* All sensitive operations (SPAKE2 math, key derivation, URSK storage) **must occur inside SE**. MCU never sees private values.
* The MCU acts as a dumb pipe with minimal logic and state.
* Each APDU exchange must be verified for response integrity (SW1/SW2, expected length).
* SE must never expose URSK, derived keys, or internal SPAKE2 state to MCU.

---

### 8. What happens after successful pairing

Once the SE marks pairing as “Owner-Paired”:

* The vehicle can enter *sharing* or *provisioning* mode to issue new keys to secondary devices.
* The SE’s secure storage includes:

  * Owner record with URSK and derived keys
  * Digital Key IDs and usage counters
  * Certificate handles and pairing metadata

---

### 9. Implementation path for you

1. **Finalize SEInCarService:**
   Implement the code skeleton I gave you for SPAKE2 request/verify.
2. **Integrate real crypto:**
   Replace mocked SPAKE2 with a proper SPAKE2+ implementation (CCC-compliant parameters: P-256, HKDF-SHA256).
3. **Implement `se_hw_transceive()`:**
   Wire up to your SE interface (I²C, SPI, ISO7816, or NFC).
4. **Test handshake:**
   Use known SPAKE2+ test vectors or CCC sample traces.
5. **Add persistent record storage:**
   Store pairing success flag in SE (APDU `STORE_PAIRING_RECORD` or vendor-specific).
6. **Hook it into your main Access Controller logic:**
   Call `Se_OwnerPairing_Perform()` during vehicle setup mode.

