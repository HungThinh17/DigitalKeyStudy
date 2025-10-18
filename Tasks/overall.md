[Phone] ⇄ (BLE/NFC)
     ↓
[Host App]
   ├── controls flows (pairing, sharing, unlock)
   ├── maintains context/state
   └── calls ↓
[SeInCarService]
   ├── stateless, blocking APIs
   ├── one call = one APDU
   └── returns parsed data only
[Secure Element]
   └── performs atomic crypto & state updates
