#include "ApduCommands.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ApduResponse make_dummy_response(const char *cmd)
{
    printf("[APDU] Executing: %s\n", cmd);
    ApduResponse r;
    r.status = 0x9000;
    r.data = (uint8_t *)strdup("DUMMY_RESPONSE");
    r.length = (uint16_t)strlen((char *)r.data);
    return r;
}

ApduResponse apdu_select(const Aid *aid)
{
    return make_dummy_response("SELECT");
}

ApduResponse apdu_spake2_request(const Spake2RequestParam *param)
{
    return make_dummy_response("SPAKE2+ REQUEST");
}

ApduResponse apdu_spake2_verify(const Spake2VerifyParam *param)
{
    return make_dummy_response("SPAKE2+ VERIFY");
}

ApduResponse apdu_write_data(uint8_t p1_flag, const Tlv *data, const uint8_t *mac, uint8_t mac_len)
{
    return make_dummy_response("WRITE DATA");
}

ApduResponse apdu_get_data(uint16_t tag, const uint8_t *mac, uint8_t mac_len)
{
    return make_dummy_response("GET DATA");
}

ApduResponse apdu_get_response(const uint8_t *mac, uint8_t mac_len)
{
    return make_dummy_response("GET RESPONSE");
}

ApduResponse apdu_op_control_flow(uint8_t p1, uint8_t p2)
{
    return make_dummy_response("OP CONTROL FLOW");
}

ApduResponse apdu_create_endpoint(const EndpointConfig *config)
{
    return make_dummy_response("CREATE ENDPOINT");
}

ApduResponse apdu_setup_endpoint(const EndpointConfig *config)
{
    return make_dummy_response("SETUP ENDPOINT");
}

ApduResponse apdu_authorize_endpoint(const Tlv *authReq)
{
    return make_dummy_response("AUTHORIZE ENDPOINT");
}

ApduResponse apdu_setup_instance(const InstanceConfig *config)
{
    return make_dummy_response("SETUP INSTANCE");
}

ApduResponse apdu_terminate_endpoint(uint16_t endpoint_id)
{
    return make_dummy_response("TERMINATE ENDPOINT");
}

ApduResponse apdu_delete_endpoint(uint16_t endpoint_id)
{
    return make_dummy_response("DELETE ENDPOINT");
}

ApduResponse apdu_convert_endpoint(const Tlv *req)
{
    return make_dummy_response("CONVERT ENDPOINT");
}

ApduResponse apdu_create_encryption_key(const KeyCreateParam *param)
{
    return make_dummy_response("CREATE ENCRYPTION KEY");
}

ApduResponse apdu_create_ranging_key(const RangingKeyParam *param)
{
    return make_dummy_response("CREATE RANGING KEY");
}

ApduResponse apdu_delete_ranging_keys(const uint8_t *key_id, uint16_t len)
{
    return make_dummy_response("DELETE RANGING KEYS");
}
ApduResponse apdu_sign(const SignDataParam *param)
{
    return make_dummy_response("SIGN");
}

ApduResponse apdu_get_private_data(const PrivateDataParam *param)
{
    return make_dummy_response("GET PRIVATE DATA");
}

ApduResponse apdu_set_private_data(const PrivateDataParam *param)
{
    return make_dummy_response("SET PRIVATE DATA");
}

ApduResponse apdu_set_confidential_data(const ConfidentialDataParam *param)
{
    return make_dummy_response("SET CONFIDENTIAL DATA");
}

ApduResponse apdu_write_buffer(uint16_t offset, const uint8_t *data, uint16_t len)
{
    return make_dummy_response("WRITE BUFFER");
}

ApduResponse apdu_read_buffer(uint16_t offset, uint16_t len)
{
    return make_dummy_response("READ BUFFER");
}

ApduResponse apdu_auth0(const AuthChallengeParam *param)
{
    return make_dummy_response("AUTH0");
}

ApduResponse apdu_auth1(const AuthProofParam *param)
{
    return make_dummy_response("AUTH1");
}

ApduResponse apdu_presence0(const AuthChallengeParam *param)
{
    return make_dummy_response("PRESENCE0");
}

ApduResponse apdu_presence1(const PresenceProofParam *param)
{
    return make_dummy_response("PRESENCE1");
}

ApduResponse apdu_exchange(const ExchangeParam *param)
{
    return make_dummy_response("EXCHANGE");
}

ApduResponse apdu_get_notification(void)
{
    return make_dummy_response("GET NOTIFICATION");
}
