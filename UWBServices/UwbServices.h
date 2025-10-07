/**
 * @file UwbService.h
 * @brief Ultra-Wideband (UWB) communication service interface
 *        for CCC Digital Key v4.0.0.
 *
 * Provides an abstraction for secure ranging session management
 * and data transport between Vehicle and Device according to
 * CCC specification:
 *   - Section 4.3: UWB Interface
 *   - Appendix G: UWB Message Framing and Flow
 *   - Table G-1 to G-4: UWB Session Control and Data Exchange
 */

#ifndef UWB_SERVICE_H
#define UWB_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "apduTypes.h"

/** UWB Role */
typedef enum {
    UWB_ROLE_VEHICLE_INITIATOR, /**< Vehicle starts ranging session */
    UWB_ROLE_DEVICE_RESPONDER   /**< Device responds to ranging requests */
} UwbRole;

/** UWB Link State */
typedef enum {
    UWB_STATE_IDLE,           /**< Not initialized */
    UWB_STATE_INITIALIZED,    /**< Radio and configuration ready */
    UWB_STATE_SESSION_OPEN,   /**< Ranging session established */
    UWB_STATE_RANGING_ACTIVE  /**< Active distance measurement ongoing */
} UwbState;

/** UWB Message Type (Appendix G Table G-2/G-3) */
typedef enum {
    UWB_MSG_CAPDU_CMD  = 0x00, /**< Command APDU data */
    UWB_MSG_RAPDU_RSP  = 0x01, /**< Response APDU data */
    UWB_MSG_ACK        = 0x10, /**< Positive acknowledgment */
    UWB_MSG_NACK       = 0x20, /**< Negative acknowledgment */
    UWB_MSG_RANGING    = 0x30  /**< Ranging measurement data */
} UwbMessageType;

/** UWB Session Information */
typedef struct {
    uint32_t sessionId;       /**< Session identifier (per CCC spec 4.3.2.2) */
    uint8_t *sessionKey;      /**< Pointer to session key (URSK or derived) */
    uint16_t keyLen;          /**< Length of session key */
    uint8_t channel;          /**< UWB channel number */
    uint8_t rangingInterval;  /**< Ranging interval (ms) */
} UwbSessionConfig;

/** UWB Encapsulation Frame */
typedef struct {
    uint8_t header;           /**< Frame type (CMD/RSP/ACK/RANGING) */
    uint16_t sequence;        /**< Sequence number */
    uint8_t *payload;         /**< Encapsulated APDU or ranging data */
    uint16_t payloadLen;      /**< Payload length */
} UwbEncapsulationFrame;

/**
 * @brief Initialize UWB subsystem with role and channel.
 *
 * @param role UWB communication role (initiator/responder)
 * @param channel Physical channel number for ranging
 * @return true if successfully initialized
 *
 * Reference: Section 4.3.2.1 UWB Initialization
 */
bool UwbService_Init(UwbRole role, uint8_t channel);

/**
 * @brief Open a secure UWB ranging session.
 *
 * Performs configuration exchange and session key setup (URSK).
 *
 * @param session Pointer to session configuration structure
 * @return true if session opened successfully
 *
 * Reference: Section 4.3.2.2 Session Establishment
 */
bool UwbService_OpenSession(const UwbSessionConfig *session);

/**
 * @brief Send encapsulated command or ranging data over UWB.
 *
 * @param frame Encapsulation frame to send
 * @return true if transmission succeeded
 *
 * Reference: Appendix G.3 ENCAPSULATED_COMMAND
 */
bool UwbService_Send(const UwbEncapsulationFrame *frame);

/**
 * @brief Receive encapsulated response or ranging data from remote side.
 *
 * @param frame Output frame buffer for received data
 * @return true if valid frame received
 *
 * Reference: Appendix G.4 ENCAPSULATED_RESPONSE
 */
bool UwbService_Receive(UwbEncapsulationFrame *frame);

/**
 * @brief Send ACK/NACK frame for previously received message.
 *
 * @param type Message type (ACK or NACK)
 * @param sequence Sequence number to acknowledge
 *
 * Reference: Appendix G.5 ACK/NACK Procedures
 */
void UwbService_SendAck(UwbMessageType type, uint16_t sequence);

/**
 * @brief Trigger or handle a ranging measurement exchange.
 *
 * @return true if ranging data successfully acquired
 *
 * Reference: Section 4.3.3 Ranging Procedure
 */
bool UwbService_PerformRanging(void);

/**
 * @brief Retrieve last ranging distance value (in centimeters).
 *
 * @return Last measured range distance or -1 if invalid.
 */
int32_t UwbService_GetLastDistance(void);

/**
 * @brief Close active UWB session and cleanup resources.
 *
 * Reference: Section 4.3.5 Session Teardown
 */
void UwbService_CloseSession(void);

/**
 * @brief Reset UWB module after error or timeout.
 *
 * Reference: Appendix G.6 Error Handling
 */
void UwbService_Reset(void);

/**
 * @brief Get current UWB state.
 *
 * @return Current UWB service state.
 */
UwbState UwbService_GetState(void);

#endif /* UWB_SERVICE_H */
