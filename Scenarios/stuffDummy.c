/**
 * @file stuffDummy.c
 * @brief Mock data and external dependencies for Owner Pairing.
 *
 * Represents device-side data, OEM provisioning, and persistent storage.
 */

#include "stuffDummy.h"
#include <stdio.h>
#include <string.h>

/* Mock SPAKE2+ keys */
static DummyKeys g_keys = {
    .devicePub = {0x11,0x22,0x33,0x44,0x55,0x66},
    .deviceProof = {0xAA,0xBB,0xCC,0xDD,0xEE,0xFF}
};

/* Mock owner provisioning data */
static DummyOwnerProvision g_prov = {
    .ownerId = 0xDEADBEEF,
    .provData = {0x10,0x20,0x30,0x40,0x50},
    .dataLen = 5
};

void StuffDummy_GetSpake2Keys(DummyKeys *out)
{
    memcpy(out, &g_keys, sizeof(DummyKeys));
}

void StuffDummy_GetOwnerProvision(DummyOwnerProvision *out)
{
    memcpy(out, &g_prov, sizeof(DummyOwnerProvision));
}

void StuffDummy_StoreVehicleOwner(uint32_t ownerId)
{
    printf("[StuffDummy] Vehicle owner ID stored: 0x%08X\n", ownerId);
}
