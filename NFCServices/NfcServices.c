/**
 * @file NFCService.c
 * @brief Dummy implementation of NFC communication service
 *        for CCC Digital Key v4.0.0.
 *
 * Provides simulated send/receive operations, frame
 * encapsulation, ACK/NACK behavior, and link setup logic
 * as defined in the CCC spec Section 3 and Appendix E.
 */

#include "NfcServices.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Local state
// -----------------------------------------------------------------------------
static NfcTechType g_currentTech = NFC_TECH_A;
static bool g_linkActive = false;

// -----------------------------------------------------------------------------
// Internal helper: create printable hex dump
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
bool NFCService_Init(NfcTechType tech)
{
    g_currentTech = tech;
    g_linkActive = false;

    const char *name = (tech == NFC_TECH_A) ? "NFC-A" :
                       (tech == NFC_TECH_B) ? "NFC-B" : "NFC-F";

    printf("[NFC] Initialized using %s interface\n", name);
    return true;
}

bool NFCService_Connect(const NfcId2 *id2)
{
    if (g_linkActive) {
        printf("[NFC] Link already active\n");
        return true;
    }

    printf("[NFC] Establishing connection...\n");

    if (g_currentTech == NFC_TECH_F && id2) {
        printf("[NFC] Target NFCID2: ");
        print_hex(id2->id, sizeof(id2->id));
    }

    g_linkActive = true;
    printf("[NFC] NFC link established successfully\n");
    return true;
}

bool NFCService_Send(const NfcEncapsulationFrame *frame)
{
    if (!g_linkActive) {
        printf("[NFC] ERROR: link not active\n");
        return false;
    }

    printf("[NFC] >>> Sending Frame\n");
    printf("  CLA: 0x%02X, P1: 0x%02X, PayloadLen: %u\n",
           frame->cla, frame->p1, frame->payloadLen);
    printf("  NFCID2: ");
    print_hex(frame->nfcid2.id, sizeof(frame->nfcid2.id));

    if (frame->payload && frame->payloadLen > 0) {
        printf("  Payload: ");
        print_hex(frame->payload, frame->payloadLen);
    } else {
        printf("  (no payload)\n");
    }

    printf("[NFC] Frame transmitted successfully (dummy)\n");
    return true;
}

bool NFCService_Receive(NfcEncapsulationFrame *frame)
{
    if (!g_linkActive) {
        printf("[NFC] ERROR: link not active\n");
        return false;
    }

    printf("[NFC] <<< Waiting for incoming frame...\n");

    // Dummy response frame
    frame->cla = 0xC3;  // Response
    frame->p1  = 0x00;  // Normal RAPDU data
    memset(frame->nfcid2.id, 0xAA, sizeof(frame->nfcid2.id));

    static const char resp[] = "DUMMY_RAPDU_RESPONSE";
    frame->payloadLen = sizeof(resp) - 1;
    frame->payload = (uint8_t*)malloc(frame->payloadLen);
    memcpy(frame->payload, resp, frame->payloadLen);

    printf("[NFC] Received dummy RAPDU frame\n");
    return true;
}

void NFCService_SendAck(NfcMessageType type, const NfcId2 *id2)
{
    if (!g_linkActive) {
        printf("[NFC] ERROR: link not active\n");
        return;
    }

    const char *msgType =
        (type == NFC_MSG_RW_ACK)     ? "RW_ACK" :
        (type == NFC_MSG_DEVICE_ACK) ? "DEVICE_ACK" :
        (type == NFC_MSG_NACK)       ? "NACK" : "UNKNOWN";

    printf("[NFC] >>> Sending %s\n", msgType);

    if (id2) {
        printf("  Target NFCID2: ");
        print_hex(id2->id, sizeof(id2->id));
    }

    printf("[NFC] %s sent (dummy)\n", msgType);
}

void NFCService_Disconnect(void)
{
    if (!g_linkActive) {
        printf("[NFC] No active link to disconnect\n");
        return;
    }

    printf("[NFC] Closing NFC link...\n");
    g_linkActive = false;
    printf("[NFC] NFC link closed\n");
}

void NFCService_Reset(void)
{
    printf("[NFC] Resetting NFC service...\n");
    g_linkActive = false;
    printf("[NFC] Link state cleared; reinitialization required\n");
}
