#ifndef STUFF_DUMMY_H
#define STUFF_DUMMY_H

#include <stdint.h>
#include "ApduTypes.h"

typedef struct {
    uint8_t devicePub[32];
    uint8_t deviceProof[32];
} DummyKeys;

typedef struct {
    uint32_t ownerId;
    uint8_t provData[64];
    uint16_t dataLen;
} DummyOwnerProvision;

void StuffDummy_GetSpake2Keys(DummyKeys *out);
void StuffDummy_GetOwnerProvision(DummyOwnerProvision *out);
void StuffDummy_StoreVehicleOwner(uint32_t ownerId);

#endif /* STUFF_DUMMY_H */
