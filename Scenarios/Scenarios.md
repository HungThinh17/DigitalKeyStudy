
| **Category**                         | **Main Scenario Name**              | **Description (from CCC spec)**                                                                                                    |
| ------------------------------------ | ----------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------- |
| **1. Pairing & Initialization**      | **Owner Pairing**                   | The process by which an owner device pairs with a vehicle for the first time, establishing the owner Digital Key. (Section 13, 14) |
|                                      | **Owner Device Change**             | The owner replaces their device and re-establishes ownership using migration or re-pairing procedures.                             |
|                                      | **Fleet In-Fleeting**               | The process by which a vehicle is enrolled under a fleet management system (FMS) or SBOD.                                          |
|                                      | **Fleet De-Fleeting**               | The process to remove a vehicle from the fleet and revoke the corresponding owner or shared keys.                                  |
| **2. Digital Key Sharing**           | **Key Sharing (Sender → Receiver)** | Owner or receiver device shares a Digital Key securely via the OEM relay or proprietary channel. (Section 11)                      |
|                                      | **Sharing in a Chain (SiaC)**       | Extended sharing model allowing delegated re-sharing while maintaining key lineage and entitlements.                               |
| **3. Key Usage (Transaction Flows)** | **Standard Transaction**            | Full NFC or BLE-based transaction with authentication and command exchanges (SELECT → AUTH0 → AUTH1 → CONTROL FLOW). (Section 15)  |
|                                      | **Fast Transaction**                | Optimized form for quick access (e.g., NFC tap-to-unlock or BLE short-range unlock).                                               |
|                                      | **Passive Entry / Passive Start**   | BLE + UWB scenario for proximity-based access and engine start. (Section 19)                                                       |
|                                      | **RKE (Remote Keyless Entry)**      | Backup or alternative access via BLE when passive entry is unavailable.                                                            |
|                                      | **Secure Ranging (UWB)**            | UWB distance measurement between device and vehicle for authentication of presence. (Section 20)                                   |
| **4. Key Lifecycle Management**      | **Key Termination**                 | Revocation or deletion of Digital Keys initiated by vehicle, device, or OEM servers. (Section 13.1–13.7)                           |
|                                      | **Key Suspension / Resume**         | Temporary disabling and reactivation of keys, e.g., when a device is lost or stolen.                                               |
|                                      | **Key Migration**                   | Moving keys across framework versions (V1 → V3) or between roles. (Section 2.10.11, 2.10.12)                                       |
| **5. Authentication & Privacy**      | **SPAKE2+ Authentication**          | ECC-based mutual authentication used during NFC and BLE sessions. (Section 14)                                                     |
|                                      | **Ephemeral Key Exchange**          | Exchange and verification of ephemeral keys for session setup.                                                                     |
|                                      | **Privacy-Protected Transactions**  | Ensuring user and vehicle data confidentiality and pseudonymization during communication.                                          |
| **6. Fleet & Service Operations**    | **Garage / Service Mode**           | Temporary transfer of access for maintenance.                                                                                      |
|                                      | **Vehicle Unpairing**               | Complete reset and deletion of Digital Keys when sold or transferred.                                                              |
|                                      | **Security Breach Handling**        | Revocation and cleanup after detected intrusion or misuse.                                                                         |
| **7. Connectivity Management**       | **Bluetooth Connection Handling**   | BLE advertisement, connection prioritization, and preference management when multiple vehicles are nearby. (Section 19.6–19.7)     |
|                                      | **Device Preference Management**    | Automatic selection and reconnection to preferred vehicle when multiple are in range.                                              |
| **8. Secure Element Operations**     | **APDU Command Sequences**          | SE applet command flows for SELECT, AUTH, CONTROL, SETUP, and data management. (Section 15.3.2)                                    |
|                                      | **Secure Channel (SCP03/SCP11)**    | Secure communication channel setup between DK framework and SE.                                                                    |

These represent the **core operational and functional scenarios** covered by the CCC v4.0.0 specification — all other sections (like error handling or subevent recovery) are sub-parts of these.
