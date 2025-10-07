/**
 * @file BleService.c
 * @brief Dummy implementation of BLE service interface for CCC Digital Key v4.0.0.
 *
 * Provides simulation of BLE GATT-based communication and encapsulation
 * logic for Digital Key APDU and ACK/NACK messages.
 */

#include "BleService.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Local state
// -----------------------------------------------------------------------------
static BleRole g_role = BLE_ROLE_VEHICLE_PERIPHERAL;
static BleState g_state = BLE_STATE_DISCONNECTED;
static uint16_t g_seqCounter = 0;

// -----------------------------------------------------------------------------
// Internal helper functions
// -----------------------------------------------------------------------------
static void print_hex(const uint8_t *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
        printf("%02X ", data[i]);
    printf("\n");
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------
bool BleService_Init(BleRole role)
{
    g_role = role;
    g_state = BLE_STATE_DISCONNECTED;
    g_seqCounter = 0;

    const char *roleName = (role == BLE_ROLE_DEVICE_CENTRAL)
                           ? "Device (Central)"
                           : "Vehicle (Peripheral)";
    printf("[BLE] Initialized as %s\n", roleName);
    return true;
}

bool BleService_Start(void)
{
    printf("[BLE] Starting %s...\n",
           (g_role == BLE_ROLE_VEHICLE_PERIPHERAL) ? "advertising" : "scanning");
    return true;
}

bool BleService_Connect(void)
{
    printf("[BLE] Establishing BLE connection...\n");
    g_state = BLE_STATE_CONNECTED;
    printf("[BLE] Connection established (dummy)\n");

    // Simulate security establishment
    g_state = BLE_STATE_SECURE_ESTABLISHED;
    printf("[BLE] Secure channel established\n");
    return true;
}

bool BleService_Send(const BleEncapsulationFrame *frame)
{
    if (g_state != BLE_STATE_SECURE_ESTABLISHED) {
        printf("[BLE] ERROR: Link not secure/connected\n");
        return false;
    }

    printf("[BLE] >>> Sending frame (Seq=%u, Type=0x%02X, Len=%u)\n",
           frame->sequence, frame->header, frame->payloadLen);
    if (frame->payload && frame->payloadLen)
        print_hex(frame->payload, frame->payloadLen);
    else
        printf("  (no payload)\n");

    printf("[BLE] Frame transmitted successfully (dummy)\n");
    return true;
}

bool BleService_Receive(BleEncapsulationFrame *frame)
{
    if (g_state != BLE_STATE_SECURE_ESTABLISHED) {
        printf("[BLE] ERROR: Link not secure/connected\n");
        return false;
    }

    printf("[BLE] <<< Waiting for response...\n");

    // Dummy data for simulation
    static const char dummyRsp[] = "BLE_RAPDU_RESPONSE";
    frame->header = BLE_MSG_RAPDU_RSP;
    frame->sequence = ++g_seqCounter;
    frame->payloadLen = sizeof(dummyRsp) - 1;
    frame->payload = (uint8_t*)malloc(frame->payloadLen);
    memcpy(frame->payload, dummyRsp, frame->payloadLen);

    printf("[BLE] Received dummy RAPDU response (Seq=%u)\n", frame->sequence);
    return true;
}

void BleService_SendAck(BleMessageType type, uint16_t sequence)
{
    const char *msgType = (type == BLE_MSG_ACK) ? "ACK" : "NACK";
    printf("[BLE] >>> Sending %s for Seq=%u\n", msgType, sequence);
    printf("[BLE] %s sent (dummy)\n", msgType);
}

BleState BleService_GetState(void)
{
    return g_state;
}

void BleService_Disconnect(void)
{
    if (g_state == BLE_STATE_DISCONNECTED) {
        printf("[BLE] Already disconnected\n");
        return;
    }

    printf("[BLE] Disconnecting BLE link...\n");
    g_state = BLE_STATE_DISCONNECTED;
    printf("[BLE] BLE link disconnected\n");
}

void BleService_Reset(void)
{
    printf("[BLE] Resetting BLE link layer...\n");
    g_state = BLE_STATE_DISCONNECTED;
    g_seqCounter = 0;
    printf("[BLE] Reset complete\n");
}
