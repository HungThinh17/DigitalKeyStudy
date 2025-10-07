#ifndef APDU_COMMAND_H
#define APDU_COMMAND_H

#include <stdint.h>
#include "ApduTypes.h"


// ============================================================
//                  CORE COMMANDS
// ============================================================

/**
 * @brief SELECT Command (see CCC Table 5-3, Section 15.3.2.1)
 *
 * Used by the host to select the Digital Key Applet instance
 * on the Secure Element before any other APDU operation.
 *
 * @param aid  Application Identifier (AID) of the target applet.
 * @return APDU response with status word only.
 */
ApduResponse apdu_select(const Aid *aid);

/**
 * @brief SPAKE2+ REQUEST Command (Table 5-4)
 *
 * Initiates SPAKE2+ key exchange used during provisioning or
 * initial pairing between Device and Vehicle Secure Elements.
 *
 * @param param Structure containing brand, firmware list, and scrypt parameters.
 * @return SPAKE2+ ephemeral public key and salt in the response.
 */
ApduResponse apdu_spake2_request(const Spake2RequestParam *param);

/**
 * @brief SPAKE2+ VERIFY Command (Table 5-7)
 *
 * Second step of SPAKE2+ handshake verifying both sides’
 * knowledge of the shared secret and generating the session seed.
 *
 * @param param Structure with peer curve point and evidence message.
 * @return Verification result and derived session seed.
 */
ApduResponse apdu_spake2_verify(const Spake2VerifyParam *param);

/**
 * @brief WRITE DATA Command (Table 5-11)
 *
 * Transfers TLV-encoded provisioning or personalization data
 * to the Secure Element, optionally protected by MAC.
 *
 * @param p1_flag  Control flag (continuation, final block, etc.)
 * @param data     TLV structure with payload content.
 * @param mac      Optional message authentication code.
 * @param mac_len  Length of MAC field.
 * @return Status indicating success or security failure.
 */
ApduResponse apdu_write_data(uint8_t p1_flag, const Tlv *data,
                             const uint8_t *mac, uint8_t mac_len);

/**
 * @brief GET DATA Command (Table 5-15)
 *
 * Reads TLV objects such as certificates or configuration
 * from the Secure Element.
 *
 * @param tag      Object tag identifier to retrieve.
 * @param mac      Optional MAC value (if secure messaging used).
 * @param mac_len  Length of MAC field.
 * @return Requested TLV object data.
 */
ApduResponse apdu_get_data(uint16_t tag, const uint8_t *mac, uint8_t mac_len);

/**
 * @brief GET RESPONSE Command (Table 5-20)
 *
 * Used when a previous command response exceeds Le.
 *
 * @param mac      Optional MAC field.
 * @param mac_len  Length of MAC.
 * @return Continuation of previous response.
 */
ApduResponse apdu_get_response(const uint8_t *mac, uint8_t mac_len);

/**
 * @brief OP CONTROL FLOW Command (Section 5.1.7)
 *
 * Synchronizes command sequencing or aborts current operation.
 *
 * @param p1  Operation control code.
 * @param p2  Optional operation qualifier.
 * @return Status code only.
 */
ApduResponse apdu_op_control_flow(uint8_t p1, uint8_t p2);


// ============================================================
//                  ENDPOINT / INSTANCE MANAGEMENT
// ============================================================

/**
 * @brief CREATE ENDPOINT Command (15.3.2.3)
 *
 * Creates a new Endpoint object (Digital Key) in the Device SE.
 *
 * @param config  Endpoint configuration parameters in TLV form.
 * @return Response containing endpoint identifier or status.
 */
ApduResponse apdu_create_endpoint(const EndpointConfig *config);

/**
 * @brief SETUP ENDPOINT Command (15.3.2.4)
 *
 * Configures the newly created Endpoint (e.g. adds certificates,
 * policy, or key material).
 *
 * @param config  Endpoint setup parameters.
 * @return Status only.
 */
ApduResponse apdu_setup_endpoint(const EndpointConfig *config);

/**
 * @brief AUTHORIZE ENDPOINT Command (15.3.2.5)
 *
 * Authorizes or validates the endpoint before activation.
 *
 * @param authReq  Authorization request TLV block.
 * @return Status or attestation proof.
 */
ApduResponse apdu_authorize_endpoint(const Tlv *authReq);

/**
 * @brief SETUP INSTANCE Command (15.3.2.2)
 *
 * Configures applet instance parameters on SE.
 *
 * @param config  Instance configuration TLV parameters.
 * @return Status only.
 */
ApduResponse apdu_setup_instance(const InstanceConfig *config);

/**
 * @brief TERMINATE ENDPOINT Command (15.3.2.6)
 *
 * Deactivates the specified endpoint.
 *
 * @param endpoint_id  Numeric endpoint identifier.
 * @return Status only.
 */
ApduResponse apdu_terminate_endpoint(uint16_t endpoint_id);

/**
 * @brief DELETE ENDPOINT Command (15.3.2.7)
 *
 * Permanently removes the endpoint object from SE storage.
 *
 * @param endpoint_id  Identifier of endpoint to delete.
 * @return Status only.
 */
ApduResponse apdu_delete_endpoint(uint16_t endpoint_id);

/**
 * @brief CONVERT ENDPOINT Command (15.3.2.8)
 *
 * Converts an existing endpoint to a key-sharing capable form.
 *
 * @param req  Conversion request TLV structure.
 * @return Updated endpoint certificate or confirmation.
 */
ApduResponse apdu_convert_endpoint(const Tlv *req);


// ============================================================
//                  KEY MANAGEMENT
// ============================================================

/**
 * @brief CREATE ENCRYPTION KEY Command (15.3.2.9)
 *
 * Generates new symmetric encryption key within SE.
 *
 * @param param  Key creation parameters (key type, length, usage flags).
 * @return Key handle or attestation data.
 */
ApduResponse apdu_create_encryption_key(const KeyCreateParam *param);

/**
 * @brief CREATE RANGING KEY Command (15.3.2.10)
 *
 * Creates UWB Ranging Key material bound to endpoint.
 *
 * @param param  Ranging key request parameters.
 * @return Derived URSK or confirmation status.
 */
ApduResponse apdu_create_ranging_key(const RangingKeyParam *param);

/**
 * @brief DELETE RANGING KEYS Command (15.3.2.11)
 *
 * Deletes one or more UWB Ranging Keys identified by key ID.
 *
 * @param key_id  Key identifier array.
 * @param len     Length of identifier array.
 * @return Status only.
 */
ApduResponse apdu_delete_ranging_keys(const uint8_t *key_id, uint16_t len);

/**
 * @brief SIGN Command (15.3.2.12)
 *
 * Signs provided input data with SE private key.
 *
 * @param param  Data to sign.
 * @return Signature output.
 */
ApduResponse apdu_sign(const SignDataParam *param);


// ============================================================
//                  DATA ACCESS
// ============================================================

/**
 * @brief GET PRIVATE DATA Command (15.3.2.18)
 *
 * Reads data from endpoint private storage area.
 *
 * @param param  Key identifier, offset, and size.
 * @return Private data TLV.
 */
ApduResponse apdu_get_private_data(const PrivateDataParam *param);

/**
 * @brief SET PRIVATE DATA Command (15.3.2.18)
 *
 * Writes or updates endpoint private data.
 *
 * @param param  Key identifier and data buffer.
 * @return Status only.
 */
ApduResponse apdu_set_private_data(const PrivateDataParam *param);

/**
 * @brief SET CONFIDENTIAL DATA Command (Mailbox write)
 *
 * Writes confidential mailbox content to SE.
 *
 * @param param  Confidential data structure.
 * @return Status only.
 */
ApduResponse apdu_set_confidential_data(const ConfidentialDataParam *param);

/**
 * @brief WRITE BUFFER Command (15.3.2.14)
 *
 * Writes temporary data to SE internal RAM buffer.
 *
 * @param offset  Write offset within SE buffer.
 * @param data    Data buffer.
 * @param len     Length of data.
 * @return Status only.
 */
ApduResponse apdu_write_buffer(uint16_t offset, const uint8_t *data, uint16_t len);

/**
 * @brief READ BUFFER Command (15.3.2.14)
 *
 * Reads data from SE temporary buffer.
 *
 * @param offset  Read offset.
 * @param len     Number of bytes to read.
 * @return Requested buffer data.
 */
ApduResponse apdu_read_buffer(uint16_t offset, uint16_t len);


// ============================================================
//                  AUTHENTICATION / PRESENCE
// ============================================================

/**
 * @brief AUTH0 Command (15.3.2.20)
 *
 * First step of mutual authentication between Device SE and Vehicle.
 *
 * @param param  Challenge data from counterpart.
 * @return Ephemeral public key or challenge response.
 */
ApduResponse apdu_auth0(const AuthChallengeParam *param);

/**
 * @brief AUTH1 Command (15.3.2.21)
 *
 * Second step of authentication, verifying proof and deriving session key.
 *
 * @param param  Proof message (MAC + encrypted data).
 * @return Verification result or derived session info.
 */
ApduResponse apdu_auth1(const AuthProofParam *param);

/**
 * @brief PRESENCE0 Command (15.3.2.22)
 *
 * Phase 1 of proximity presence check.
 *
 * @param param  Vehicle challenge data.
 * @return MAC or partial presence proof.
 */
ApduResponse apdu_presence0(const AuthChallengeParam *param);

/**
 * @brief PRESENCE1 Command (15.3.2.23)
 *
 * Phase 2 of presence check finalizing session validation.
 *
 * @param param  Signature proof of proximity.
 * @return Encrypted response payload.
 */
ApduResponse apdu_presence1(const PresenceProofParam *param);


// ============================================================
//                  EXCHANGE / NOTIFICATION
// ============================================================

/**
 * @brief EXCHANGE Command (15.3.2.15)
 *
 * Secure data exchange channel post-authentication.
 *
 * @param param  Encrypted payload structure.
 * @return Encrypted response.
 */
ApduResponse apdu_exchange(const ExchangeParam *param);

/**
 * @brief GET NOTIFICATION Command (15.3.2.24)
 *
 * Retrieves pending notifications or events generated by
 * previous APDU operations.
 *
 * @return Notification TLV list or empty status.
 */
ApduResponse apdu_get_notification(void);

#endif // APDU_COMMAND_H
