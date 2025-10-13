# SeInCarService – Secure Element Basic Service

### Overview

This module provides a **bare-metal**, **blocking** interface for communicating with an in-car Secure Element (SE) such as NXP NCJ38A, compliant with the CCC Digital Key specification.
It handles APDU build/serialize/send/parse, abstracts hardware through `SEHostLib`, and uses a simple internal state machine (`UNINIT → IDLE → BUSY → ERROR`).

### Build integration

Add these files to your build and include the header:

```c
#include "SeInCarServices.h"
```

The mock library can be used for PC or early firmware testing; replace it later with a real SEHost driver (SPI/I2C).

---

### Typical call flow

```c
SeStatus_t st;
ApduResponse resp;

st = Se_Init();             // bring up host + context
st = Se_Reset();            // optional reset
st = Se_BeginAccess();      // mark SE busy

SeSelectParam_t sel = { .aid = NULL, .aid_len = 0, .p1 = 0x04, .p2 = 0x00 };
st = Se_SelectFramework(&sel, &resp, 100);

Se_EndAccess();             // release SE
Se_Deinit();                // close everything
```

---

### State model

| State      | Meaning                         |
| ---------- | ------------------------------- |
| **UNINIT** | Not initialized or after deinit |
| **IDLE**   | Ready to send commands          |
| **BUSY**   | APDU transaction in progress    |
| **ERROR**  | Last operation failed           |

Check readiness before each operation:

```c
if (Se_IsReady() != SE_OK) { /* handle not-ready case */ }
```

### Mock operation

The `SEHostLib_Mock.c` simulates realistic APDU behavior:

* Generates dummy data per INS code (`0xA4`, `0x30`, `0x32`, etc.).
* Delays 10–50 ms to mimic SE latency.
* Prints all transactions to stdout via `MOCK_LOG()`.

Use this mock for PC testing before wiring real hardware.

---

### Design philosophy

* **Single-channel:** one access at a time (no sessions).
* **Deterministic:** no heap allocation in core path.
* **Portable:** runs on bare metal or simple RTOS.
* **Replaceable transport:** swap `SEHostLib_Mock` with a real driver when ready.
