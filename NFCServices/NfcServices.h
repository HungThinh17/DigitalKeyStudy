/**
 * @file NFCService.h
 * @brief NFC communication service interface for CCC Digital Key (v4.0.0).
 *
 * This interface provides abstraction for NFC-A/B (ISO-DEP) and NFC-F (Type 3 Tag)
 * communication used between Vehicle NFC Reader and Device (Phone or Keyfob).
 *
 * Reference:
 *  - CCC-TS-101 Digital Key v4.0.0, Section 3 (NFC Interface)
 *  - Appendix E (NFC-F Support)
 *  - Table E-1 to E-4 (ENCAPSULATION_CMD / RSP)
 *
 * Two main participants exist:
 *  - Vehicle side: NFC Reader communicating via ISO-DEP or NFC-F
 *  - Device side: Secure Element (SE) or Digital Key Applet handling APDUs
 */

#ifndef NFC_SERVICE_H
#define NFC_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "apduTypes.h"   // Include APDU command types

/** NFC Technology Type */
typedef enum {
    NFC_TECH_A,      /**< ISO14443-A (Type 4A) */
    NFC_TECH_B,      /**< ISO14443-B (Type 4B) */
    NFC_TECH_F       /**< NFC-F (Type 3 Tag) */
} NfcTechType;

/** NFC Frame Direction */
typedef enum {
    NFC_DIR_VEHICLE_TO_DEVICE,
    NFC_DIR_DEVICE_TO_VEHICLE
} NfcDirection;

/** NFC Encapsulation Command Type (Table E-2 / E-4) */
typedef enum {
    NFC_MSG_CAPDU_DATA     = 0x00, /**< Vehicle→Device command APDU data */
    NFC_MSG_RAPDU_DATA     = 0x00, /**< Device→Vehicle response APDU data */
    NFC_MSG_RW_ACK         = 0x10, /**< Vehicle→Device ACK (no payload) */
    NFC_MSG_NACK           = 0x20, /**< Vehicle→Device negative ACK */
    NFC_MSG_DEVICE_ACK     = 0x10  /**< Device→Vehicle ACK (no payload) */
} NfcMessageType;

/** NFC ID (Type F NFCID2) */
typedef struct {
    uint8_t id[8]; /**< NFCID2 value from SENSF_RES response */
} NfcId2;

/** NFC Encapsulation frame (Appendix E.4) */
typedef struct {
    uint8_t cla;          /**< Command code: 0xC2 for CMD, 0xC3 for RSP */
    uint8_t p1;           /**< Message Type + Feature bits (see Table E-2, E-4) */
    NfcId2 nfcid2;        /**< NFCID2 to identify target */
    uint8_t *payload;     /**< APDU payload bytes */
    uint16_t payloadLen;  /**< Length of payload */
} NfcEncapsulationFrame;

/**
 * @brief Initialize the NFC service with the desired technology type.
 * @param tech Technology type (NFC-A, NFC-B, NFC-F)
 * @return true if initialization succeeded, false otherwise
 *
 * Reference: Section 3.1 NFC Functional Requirements
 */
bool NFCService_Init(NfcTechType tech);

/**
 * @brief Establish an NFC connection with a remote device.
 * @param id2 Pointer to target NFCID2 (only for NFC-F)
 * @return true if connection established successfully.
 *
 * Reference: Section 3.2 NFC Polling and Link Setup Procedure
 */
bool NFCService_Connect(const NfcId2 *id2);

/**
 * @brief Send an encapsulated APDU command to the remote side.
 * @param frame Encapsulation frame containing APDU or ACK data
 * @return true if sent successfully
 *
 * Reference: Appendix E.4.1 ENCAPSULATION_CMD
 */
bool NFCService_Send(const NfcEncapsulationFrame *frame);

/**
 * @brief Receive an encapsulated APDU response from the remote side.
 * @param frame Output buffer to store received frame
 * @return true if a valid frame is received
 *
 * Reference: Appendix E.4.2 ENCAPSULATION_RSP
 */
bool NFCService_Receive(NfcEncapsulationFrame *frame);

/**
 * @brief Send a simple ACK or NACK message (for CAPDU/RAPDU flow control).
 * @param type Message type (NFC_MSG_RW_ACK, NFC_MSG_DEVICE_ACK, NFC_MSG_NACK)
 * @param id2  NFCID2 identifying target
 *
 * Reference: Appendix E.4.1–E.4.2, RW_ACK / DEVICE_ACK behavior
 */
void NFCService_SendAck(NfcMessageType type, const NfcId2 *id2);

/**
 * @brief Close the NFC link and perform deactivation.
 * Reference: Section 3.2.3 NFC Link Teardown Procedure
 */
void NFCService_Disconnect(void);

/**
 * @brief Reset or reinitialize the NFC interface after transmission error or timeout.
 * Reference: Appendix E.1 Protocol Error / Timeout Error handling.
 */
void NFCService_Reset(void);

#endif /* NFC_SERVICE_H */
