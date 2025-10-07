#ifndef APDU_TYPES_H
#define APDU_TYPES_H

/**
 * @file apduTypes.h
 * @brief Defines parameter and data structure types used by APDU command wrappers.
 *
 * Each type corresponds to a parameter group from the CCC Digital Key v4.0.0
 * specification (Section 15.3.2.x). All fields are designed to reflect
 * the TLV-oriented nature of SE commands and responses.
 *
 * These definitions serve as a C abstraction layer for building or parsing
 * APDU command data, allowing high-level application logic to remain
 * implementation-agnostic.
 */

#include <stdint.h>


// ============================================================
//                     GENERIC DATA FORMATS
// ============================================================

/**
 * @brief Generic APDU response object returned by all commands.
 */
typedef struct {
    uint16_t status;     /**< ISO7816 status word (e.g. 0x9000 = success) */
    uint8_t *data;       /**< Pointer to returned payload, if any */
    uint16_t length;     /**< Length of returned payload in bytes */
} ApduResponse;

/**
 * @brief Generic TLV (Tag-Length-Value) representation.
 *
 * Used widely throughout the CCC specification to encode
 * dynamic data such as certificates, configuration objects,
 * or endpoint parameters.
 */
typedef struct {
    uint16_t tag;      /**< TLV Tag identifier */
    uint16_t length;   /**< Length of value field in bytes */
    uint8_t *value;    /**< Pointer to TLV value bytes */
} Tlv;

/**
 * @brief Application Identifier for SELECT command.
 *
 * Used to identify the Digital Key applet instance on the Secure Element.
 * Defined in ISO/IEC 7816-4.
 */
typedef struct {
    uint8_t *aid;      /**< Pointer to Application Identifier bytes */
    uint8_t length;    /**< Length of AID */
} Aid;


// ============================================================
//                    SPAKE2+ EXCHANGE STRUCTURES
// ============================================================

/**
 * @brief Input parameters for SPAKE2+ REQUEST command.
 *
 * Used during provisioning to initiate key agreement between
 * the Vehicle and Device Secure Elements.
 *
 * Reference: CCC Table 5-4 and Section 15.3.2.2.
 */
typedef struct {
    uint8_t *fwList;       /**< Firmware identification TLV */
    uint8_t fwListLen;     /**< Length of firmware list */
    uint8_t *txList;       /**< Transmit capability list TLV */
    uint8_t txListLen;     /**< Length of transmit list */
    uint8_t *btList;       /**< Bluetooth capability list TLV */
    uint8_t btListLen;     /**< Length of Bluetooth list */
    uint8_t *scryptParams; /**< Scrypt salt and parameters */
    uint8_t scryptLen;     /**< Length of scrypt parameters */
    uint16_t vehicleBrand; /**< Vehicle brand identifier (OEM specific) */
} Spake2RequestParam;

/**
 * @brief Input parameters for SPAKE2+ VERIFY command.
 *
 * Used for mutual authentication of shared key material between SEs.
 *
 * Reference: CCC Table 5-7.
 */
typedef struct {
    uint8_t *curveY;        /**< Peer public curve point (Y coordinate) */
    uint16_t lenY;          /**< Length of curve point */
    uint8_t *evidenceM1;    /**< Evidence message M1 proving key possession */
    uint16_t lenM1;         /**< Length of evidence message */
} Spake2VerifyParam;


// ============================================================
//                    ENDPOINT / INSTANCE DATA
// ============================================================

/**
 * @brief Endpoint configuration or setup structure.
 *
 * Used in CREATE ENDPOINT, SETUP ENDPOINT, and related commands.
 *
 * Reference: CCC Section 15.3.2.3–15.3.2.6.
 */
typedef struct {
    uint16_t endpointId;    /**< Unique endpoint identifier (assigned by SE) */
    uint8_t *configTlv;     /**< Pointer to TLV-encoded configuration data */
    uint16_t configLen;     /**< Length of configuration TLV data */
} EndpointConfig;

/**
 * @brief Instance configuration structure.
 *
 * Defines Digital Key applet instance parameters such as
 * policy control, certificate roots, and supported features.
 *
 * Reference: CCC Section 15.3.2.2.
 */
typedef struct {
    uint8_t *paramsTlv;     /**< TLV-encoded instance configuration parameters */
    uint16_t paramsLen;     /**< Length of TLV data */
} InstanceConfig;


// ============================================================
//                     KEY MANAGEMENT STRUCTURES
// ============================================================

/**
 * @brief Parameters for CREATE ENCRYPTION KEY command.
 *
 * Defines the key type, usage, and desired cryptographic attributes.
 *
 * Reference: CCC Section 15.3.2.9.
 */
typedef struct {
    uint8_t *keyParamsTlv;  /**< TLV defining key attributes (algorithm, length, usage) */
    uint16_t len;           /**< Length of TLV data */
} KeyCreateParam;

/**
 * @brief Parameters for CREATE RANGING KEY command.
 *
 * Used for generation of UWB (Ultra-Wideband) ranging keys bound to endpoint.
 *
 * Reference: CCC Section 15.3.2.10.
 */
typedef struct {
    uint8_t *endpointInfo;  /**< TLV-encoded endpoint information block */
    uint16_t len;           /**< Length of endpoint info data */
} RangingKeyParam;

/**
 * @brief Input data for SIGN command.
 *
 * Reference: CCC Section 15.3.2.12.
 */
typedef struct {
    uint8_t *data;          /**< Pointer to input data to sign */
    uint16_t len;           /**< Length of input data */
} SignDataParam;


// ============================================================
//                     PRIVATE / CONFIDENTIAL DATA
// ============================================================

/**
 * @brief Parameters for GET or SET PRIVATE DATA command.
 *
 * Used for reading or writing application-level private objects
 * such as endpoint secrets, stored as TLVs.
 *
 * Reference: CCC Section 15.3.2.18.
 */
typedef struct {
    uint8_t *keyIdentifier; /**< Identifier for private data object */
    uint8_t *data;          /**< Pointer to read/write buffer */
    uint16_t len;           /**< Length of data buffer */
    uint16_t offset;        /**< Offset within object for partial access */
} PrivateDataParam;

/**
 * @brief Parameters for SET CONFIDENTIAL DATA command.
 *
 * Used to store encrypted mailbox content or sensitive information.
 *
 * Reference: CCC Section 15.3.2.18 (Mailbox Option A).
 */
typedef struct {
    uint8_t *data;          /**< Encrypted confidential data buffer */
    uint16_t len;           /**< Length of data */
} ConfidentialDataParam;


// ============================================================
//                     AUTHENTICATION / PRESENCE
// ============================================================

/**
 * @brief Authentication challenge data used in AUTH0 or PRESENCE0 commands.
 *
 * Reference: CCC Section 15.3.2.20 and 15.3.2.22.
 */
typedef struct {
    uint8_t *challenge;     /**< Challenge or nonce from the counterpart */
    uint16_t len;           /**< Length of challenge */
} AuthChallengeParam;

/**
 * @brief Authentication proof data used in AUTH1 command.
 *
 * Reference: CCC Section 15.3.2.21.
 */
typedef struct {
    uint8_t *proof;         /**< Proof message containing MAC/encrypted data */
    uint16_t len;           /**< Length of proof data */
} AuthProofParam;

/**
 * @brief Presence proof signature data used in PRESENCE1 command.
 *
 * Reference: CCC Section 15.3.2.23.
 */
typedef struct {
    uint8_t *signature;     /**< Signature proving proximity */
    uint16_t len;           /**< Length of signature */
} PresenceProofParam;


// ============================================================
//                     SECURE DATA EXCHANGE
// ============================================================

/**
 * @brief Parameters for EXCHANGE command.
 *
 * Used for general-purpose encrypted data transfer
 * after successful authentication.
 *
 * Reference: CCC Section 15.3.2.15.
 */
typedef struct {
    uint8_t *payload;       /**< Encrypted payload to send */
    uint16_t len;           /**< Length of payload */
} ExchangeParam;

#endif // APDU_TYPES_H

