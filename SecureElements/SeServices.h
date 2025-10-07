/**
 * @file SeService.h
 * @brief In-Car Secure Element (SE) core service interface
 *        for CCC Digital Key v4.0.0.
 *
 * Purpose:
 *  - Manage in-car Secure Element lifecycle and host integration points.
 *  - Provide initialization, reset, and notification callback registration.
 *  - Maintain state of SE readiness for higher-level modules.
 *
 * Notes:
 *  - Does not implement APDU exchange or data transport.
 *  - Aligns with CCC-TS-101 Digital Key v4.0.0 Section 15 (Applet Implementation)
 *    and Section 15.3.1.9 (Notification of the Digital Key Framework).
 */

#ifndef SE_SERVICE_H
#define SE_SERVICE_H

#include <stdint.h>
#include <stdbool.h>
#include "apduTypes.h"

/* -------------------------------------------------------------------------
 * Notification callback (EVT_TRANSACTION, error, or state events)
 * ------------------------------------------------------------------------- */
/**
 * @brief SE notification callback.
 *
 * Invoked when a transaction event or system-level SE event occurs.
 *
 * @param resp Pointer to APDU response payload or event data (may be NULL)
 * @param ctx  User-defined context pointer from registration
 *
 * Reference: CCC Section 15.3.1.9 – Notification of the Digital Key framework
 */
typedef void (*SeNotificationCb)(const ApduResponse *resp, void *ctx);

/* -------------------------------------------------------------------------
 * Bus / hardware setup type
 * ------------------------------------------------------------------------- */
/** Secure Element bus type used in the vehicle. */
typedef enum {
    SE_BUS_I2C,
    SE_BUS_SPI,
    SE_BUS_UART
} SeBusType;

/** SE Bus configuration structure. */
typedef struct {
    SeBusType bus;
    union {
        struct {
            uint8_t address;
            uint32_t frequency_hz;
        } i2c;
        struct {
            int cs_pin;
            uint32_t frequency_hz;
            uint8_t mode;
        } spi;
        struct {
            uint32_t baudrate;
            uint8_t parity;
        } uart;
    } cfg;
} SeBusConfig;

/* -------------------------------------------------------------------------
 * Public API
 * ------------------------------------------------------------------------- */

/**
 * @brief Initialize the in-car Secure Element service.
 *
 * - Configures underlying hardware interface (I2C/SPI/UART).
 * - Sets up any GPIO or regulator control for SE power domain.
 * - Does not power on or communicate with SE — only prepares environment.
 *
 * @param cfg Pointer to SE bus configuration
 * @return true if initialization succeeded.
 *
 * Reference: CCC Digital Key v4.0.0 Section 15 (Applet Implementation Context)
 */
bool SEService_Init(const SeBusConfig *cfg);

/**
 * @brief Reset or reinitialize the in-car Secure Element.
 *
 * - Typically toggles SE RESET pin or sends a warm reset command.
 * - Used for fault recovery or re-synchronization with the SE.
 *
 * @return true if reset was executed successfully.
 *
 * Reference: CCC Digital Key v4.0.0 Section 15.3 (Applet Behavior)
 */
bool SEService_Reset(void);

/**
 * @brief Register or unregister callback for SE notifications.
 *
 * - Callback will be triggered when SE sends EVT_TRANSACTION or
 *   other system notifications (e.g., internal state change).
 *
 * @param cb  Function pointer to callback (NULL to unregister)
 * @param ctx User-defined context pointer returned in callback
 */
void SEService_RegisterNotificationCallback(SeNotificationCb cb, void *ctx);

/**
 * @brief Check whether the SE is initialized and ready.
 *
 * - Returns true if SEService_Init() has been called and hardware
 *   resources are valid.
 *
 * @return true if SE service is ready for use.
 */
bool SEService_IsReady(void);

#endif /* SE_SERVICE_H */
