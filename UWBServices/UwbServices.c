/**
 * @file UwbService.c
 * @brief Dummy implementation of UWB service interface
 *        for CCC Digital Key v4.0.0.
 *
 * Simulates UWB ranging session setup, message encapsulation,
 * and secure distance measurement as described in the CCC spec.
 */

#include "UwbService.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Local state
// -----------------------------------------------------------------------------
static UwbRole g_role = UWB_ROLE_VEHICLE_INITIATOR;
static UwbState g_state = UWB_STATE_IDLE;
static uint16_t g_seqCounter = 0;
static int32_t g_lastDistance = -1;

// -----------------------------------------------------------------------------
// Helpers
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
bool UwbService_Init(UwbRole role, uint8_t channel)
{
    g_role = role;
    g_state = UWB_STATE_INITIALIZED;
    g_seqCounter = 0;
    g_lastDistance = -1;

    const char *roleName = (role == UWB_ROLE_VEHICLE_INITIATOR)
                           ? "Vehicle Initiator"
                           : "Device Responder";

    printf("[UWB] Initialized (%s) on channel %u\n", roleName, channel);
    return true;
}

bool UwbService_OpenSession(const UwbSessionConfig *session)
{
    if (g_state != UWB_STATE_INITIALIZED) {
        printf("[UWB] ERROR: not initialized\n");
        return false;
    }

    printf("[UWB] Opening session ID=%u, channel=%u, interval=%ums\n",
           session->sessionId, session->channel, session->rangingInterval);
    printf("       URSK (len=%u): ", session->keyLen);
    print_hex(session->sessionKey, session->keyLen);

    g_state = UWB_STATE_SESSION_OPEN;
    printf("[UWB] UWB session opened successfully (dummy)\n");
    return true;
}

bool UwbService_Send(const UwbEncapsulationFrame *frame)
{
    if (g_state < UWB_STATE_SESSION_OPEN) {
        printf("[UWB] ERROR: session not open\n");
        return false;
    }

    printf("[UWB] >>> Sending frame (Seq=%u, Type=0x%02X, Len=%u)\n",
           frame->sequence, frame->header, frame->payloadLen);
    if (frame->payload && frame->payloadLen)
        print_hex(frame->payload, frame->payloadLen);
    else
        printf("  (no payload)\n");

    printf("[UWB] Frame transmitted successfully (dummy)\n");
    return true;
}

bool UwbService_Receive(UwbEncapsulationFrame *frame)
{
    if (g_state < UWB_STATE_SESSION_OPEN) {
        printf("[UWB] ERROR: session not open\n");
        return false;
    }

    printf("[UWB] <<< Waiting for response frame...\n");

    // Simulated response
    static const char dummyResp[] = "UWB_RAPDU_RESPONSE";
    frame->header = UWB_MSG_RAPDU_RSP;
    frame->sequence = ++g_seqCounter;
    frame->payloadLen = sizeof(dummyResp) - 1;
    frame->payload = (uint8_t*)malloc(frame->payloadLen);
    memcpy(frame->payload, dummyResp, frame->payloadLen);

    printf("[UWB] Received dummy response (Seq=%u)\n", frame->sequence);
    return true;
}

void UwbService_SendAck(UwbMessageType type, uint16_t sequence)
{
    const char *msgType = (type == UWB_MSG_ACK) ? "ACK" : "NACK";
    printf("[UWB] >>> Sending %s for Seq=%u\n", msgType, sequence);
    printf("[UWB] %s sent (dummy)\n", msgType);
}

bool UwbService_PerformRanging(void)
{
    if (g_state < UWB_STATE_SESSION_OPEN) {
        printf("[UWB] ERROR: session not open\n");
        return false;
    }

    printf("[UWB] Performing ranging exchange...\n");
    g_state = UWB_STATE_RANGING_ACTIVE;

    // Dummy distance simulation
    g_lastDistance = 127; // 1.27 meters
    printf("[UWB] Ranging complete. Distance = %.2f m\n", g_lastDistance / 100.0);

    g_state = UWB_STATE_SESSION_OPEN;
    return true;
}

int32_t UwbService_GetLastDistance(void)
{
    return g_lastDistance;
}

void UwbService_CloseSession(void)
{
    if (g_state < UWB_STATE_SESSION_OPEN) {
        printf("[UWB] No active session to close\n");
        return;
    }

    printf("[UWB] Closing UWB session...\n");
    g_state = UWB_STATE_INITIALIZED;
    printf("[UWB] UWB session closed\n");
}

void UwbService_Reset(void)
{
    printf("[UWB] Resetting UWB subsystem...\n");
    g_state = UWB_STATE_IDLE;
    g_seqCounter = 0;
    g_lastDistance = -1;
    printf("[UWB] Reset complete\n");
}

UwbState UwbService_GetState(void)
{
    return g_state;
}
