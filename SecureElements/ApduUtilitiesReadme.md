
# Quick usage examples (copy-paste)

1. Build + serialize SELECT default AID:

```c
ApduFrame frame;
if (apdu_build_command(APDU_CMD_SELECT, NULL, &frame) != 0) { /* error */ }

ApduWireBuffer wire;
if (apdu_serialize_frame(&frame, &wire) != 0) { /* error */ }

apdu_dump_wire(wire.buf, wire.len); // debug hex
// send wire.buf (wire.len) to transport...

apdu_free_wirebuffer(&wire);
apdu_free_frame(&frame);
```

2. Build SPAKE2 request with payload:

```c
uint8_t spake_blob[] = { 0x10, 0x02, 0xAA, 0xBB }; // example
ApduGenericParam p = { spake_blob, sizeof(spake_blob), 0, 0xFF };
ApduFrame spf;
apdu_build_command(APDU_CMD_SPAKE2_REQUEST, &p, &spf);
ApduWireBuffer w;
apdu_serialize_frame(&spf, &w);
apdu_dump_wire(w.buf, w.len);
apdu_free_wirebuffer(&w);
apdu_free_frame(&spf);
```

3. Wrap for DK_APDU_RQ (BLE):

```c
ApduWireBuffer wrapped;
int r = apdu_wrap_dk_apdu_rq(&w, &wrapped);
if (r == 0) {
    // send wrapped.buf/len over BLE characteristic
    apdu_free_wirebuffer(&wrapped);
} else {
    // handle error: r == -2 means CLA not allowed
}
```

4. Parse RAPDU:

```c
uint8_t rapdu_example[] = { 0xDE,0xAD,0xBE,0xEF, 0x90, 0x00 }; // data + SW1 SW2
ApduResponse resp;
if (apdu_parse_response(rapdu_example, sizeof(rapdu_example), &resp) == 0) {
    printf("SW=%02X%02X len=%u\n", resp.sw1, resp.sw2, (unsigned)resp.len);
    apdu_free_response(&resp);
}
```

