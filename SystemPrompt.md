1. APDU Command Layer (Original Implementation)

Extracted all APDU commands from the CCC spec (v4.0.0).

Implemented individual command functions (apduCommand_SELECT(), etc.).

Created apduTypes.h for shared parameter types and ApduCommands.h/.c for execution.

Added inline comments with CCC clause references.

2. NFC, BLE, and UWB Services

Designed NFCService.[ch], BleService.[ch], and UwbService.[ch].

Each exposes Init(), Start(), Connect(), Send(), Receive(), Disconnect().

All follow a common pattern and reference their CCC sections (§3, §19, §20).

These provide hardware-independent comms abstraction.

3. Secure Element (SE) Service

Built SeService.[ch] focused purely on in-car SE lifecycle.

APIs kept minimal:

SEService_Init, SEService_Reset, SEService_RegisterNotificationCallback, SEService_IsReady, SEService_Write, SEService_Read.

Avoided APDU duplication; SE only handles transport and readiness.

4. Scenario Enumeration

Identified all main system scenarios from the CCC spec:
Owner Pairing, Device Change, Fleet In/De-Fleeting, Key Sharing, Passive Entry, Secure Ranging, Key Termination, Migration, etc.

Defined these as the future task layer modules in the architecture.

5. Owner Pairing Scenario Implementation

Implemented OwnerPairingTask.[ch] as a complete vehicle-side flow:

Initialize SE.

Establish transport (NFC → BLE fallback).

Perform SPAKE2+ authentication.

Send OEM provisioning data.

Confirm ownership.

Added stuffDummy.[ch] mocks for OEM and device provisioning data.

6. NFC → BLE Fallback Mechanism

Enhanced OwnerPairingTask to prefer NFC and automatically fall back to BLE on link or authentication failure.

Transport-agnostic pairing logic ensures full CCC multi-transport compliance.

7. Framework Architecture Definition

Introduced three core framework modules:

CccFrameworkCore – service registry & event orchestration.

CccStateManager – finite-state machine enforcing CCC lifecycle.

CccErrorHandler – fault classification and recovery policy.

Defined overall system layer stack and lifecycle states (UNPAIRED → OWNER_PAIRING → OWNER_ACTIVE → …).

8. Owner Pairing Flow Within the Framework

Mapped how the pairing scenario executes across the framework:

NFCService triggers event → FrameworkCore → StateManager → OwnerPairingTask.

Task executes APDU SPAKE2+/SetupInstance via SE.

Success → OWNER_ACTIVE; Failure → rollback via ErrorHandler.

Produced full event and state flow diagram.

9. Upcoming Core Development Plan

Plan to define the Framework Event Contract:

Enumerate all event IDs (EVENT_NFC_CONNECTED, EVENT_PAIRING_COMPLETE, etc.).

Establish publisher/subscriber model between FrameworkCore, services, and tasks.

This will enable actual runtime coupling between components.

10. Transition to APDU Utilities (Refinement of Stage 1)

Decision: Replace function-per-command pattern with a data-driven APDU Utilities module.

Key Changes:

Introduced ApduCommandId enumeration listing all CCC APDU commands.

Created unified ApduFrame structure (CLA, INS, P1, P2, Lc, Data, Le).

Implemented apdu_build_command(cmd_id, params, outFrame) API.

Added centralized descriptor table mapping cmd_id to CLA/INS/P1/P2 and defaults.

Optional: apdu_parse_response() for symmetry.

Impact:

SE and transport layers (NFC/BLE) can share one binary APDU format.

Easier maintenance and cross-platform reuse (device and vehicle sides).

Clean integration with FrameworkCore’s transport routing.

Status: Adopted as the new baseline; ApduUtilities.[ch] will replace the old ApduCommands module.

Next Step

Lock down:

ApduUtilities.h (header design, enum + struct + API prototype).

Then define the Framework Event Contract to connect it with SeService, BleService, and OwnerPairingTask.