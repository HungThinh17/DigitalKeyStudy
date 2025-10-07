/**
 * @file BleService.h
 * @brief BLE communication service interface for CCC Digital Key v4.0.0.
 *
 * Provides logical encapsulation of Digital Key APDU and control messages
 * transported via BLE GATT characteristics (WCC2/WCC3).
 *
 * Reference:
 *  - CCC-TS-101 Digital Key v4.0.0
 *  - Section 4.2 BLE Interface
 *  - Appendix F: BLE Message Framing and Flow
 *  - Table F-1 to F-4 (ENCAPSULATED_COMMAND, RESPONSE, ACK, NACK)
 */

#ifndef BLE_SERVICE_H
#define BLE_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "apduTypes.h"

/** BLE Connection Role */
typedef enum {
    BLE_ROLE_VEHICLE_PERIPHERAL,  /**< Vehicle advertising, device connects */
    BLE_ROLE_DEVICE_CENTRAL       /**< Device connects to vehicle peripheral */
} BleRole;

/** BLE Channel State */
typedef enum {
    BLE_STATE_DISCONNECTED,
    BLE_STATE_CONNECTED,
    BLE_STATE_SECURE_ESTABLISHED
} BleState;

/** BLE Encapsulation Message Type (Appendix F Tables F-1/F-2) */
typedef enum {
    BLE_MSG_CAPDU_CMD  = 0x00, /**< Command APDU data (Vehicle→Device) */
    BLE_MSG_RAPDU_RSP  = 0x01, /**< Response APDU data (Device→Vehicle) */
    BLE_MSG_ACK        = 0x10, /**< Positive acknowledgement */
    BLE_MSG_NACK       = 0x20  /**< Negative acknowledgement */
} BleMessageType;

/** BLE Encapsulation Frame */
typedef struct {
    uint8_t header;           /**< Message type (CMD/RSP/ACK/NACK) */
    uint16_t sequence;        /**< Sequence counter for fragmentation control */
    uint8_t *payload;         /**< Payload (APDU fragment or ACK info) */
    uint16_t payloadLen;      /**< Length of payload */
} BleEncapsulationFrame;

/**
 * @brief Initialize BLE service and prepare GATT profiles.
 *
 * Sets up BLE stack role, UUIDs for Digital Key Service and
 * its characteristics.
 *
 * @param role BLE role (vehicle peripheral or device central)
 * @return true if initialized successfully.
 *
 * Reference: Section 4.2.2 BLE GATT Profile Initialization
 */
bool BleService_Init(BleRole role);

/**
 * @brief Start BLE advertising (for vehicle) or scanning (for device).
 *
 * @return true if operation started successfully.
 *
 * Reference: Section 4.2.3 BLE Advertising and Discovery
 */
bool BleService_Start(void);

/**
 * @brief Establish BLE connection and optionally secure link.
 *
 * @return true if connected successfully.
 *
 * Reference: Section 4.2.4 BLE Connection Establishment
 */
bool BleService_Connect(void);

/**
 * @brief Send encapsulated command frame over BLE (GATT write).
 *
 * Typically used for CAPDU data or control commands.
 *
 * @param frame Encapsulation frame to send.
 * @return true if write succeeded.
 *
 * Reference: Appendix F.3 ENCAPSULATED_COMMAND
 */
bool BleService_Send(const BleEncapsulationFrame *frame);

/**
 * @brief Receive encapsulated response frame over BLE (GATT notify/indicate).
 *
 * Used to receive RAPDU or ACK/NACK responses.
 *
 * @param frame Output frame structure to populate.
 * @return true if a valid frame received.
 *
 * Reference: Appendix F.4 ENCAPSULATED_RESPONSE
 */
bool BleService_Receive(BleEncapsulationFrame *frame);

/**
 * @brief Send acknowledgment or negative acknowledgment frame.
 *
 * @param type ACK or NACK message type.
 * @param sequence Sequence number of frame being acknowledged.
 *
 * Reference: Appendix F.5 ACK/NACK Procedures
 */
void BleService_SendAck(BleMessageType type, uint16_t sequence);

/**
 * @brief Get current BLE service state.
 *
 * @return Current BLE connection state.
 */
BleState BleService_GetState(void);

/**
 * @brief Close BLE connection and stop service.
 *
 * Reference: Section 4.2.7 BLE Link Teardown
 */
void BleService_Disconnect(void);

/**
 * @brief Reset BLE link layer and clear internal state.
 *
 * Used after connection errors, timeouts, or reinitialization events.
 *
 * Reference: Appendix F.6 BLE Error Recovery
 */
void BleService_Reset(void);

#endif /* BLE_SERVICE_H */
