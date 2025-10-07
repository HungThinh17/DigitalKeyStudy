/**
 * @file SeService.c
 * @brief Minimal implementation of in-car Secure Element (SE) service.
 *
 * Provides initialization, reset, and notification registration for the
 * vehicle’s Secure Element according to CCC Digital Key v4.0.0.
 *
 * No transport or APDU handling logic is implemented here.
 */

#include "SeServices.h"
#include <stdio.h>
#include <string.h>

/* Internal state */
static bool g_inited = false;
static SeBusConfig g_cfg = {0};
static SeNotificationCb g_notify_cb = NULL;
static void *g_notify_ctx = NULL;

bool SEService_Init(const SeBusConfig *cfg)
{
    if (!cfg) return false;
    memcpy(&g_cfg, cfg, sizeof(g_cfg));
    g_inited = true;

    const char *busName = (cfg->bus == SE_BUS_I2C) ? "I2C" :
                          (cfg->bus == SE_BUS_SPI) ? "SPI" : "UART";
    printf("[SEService] Initialized on %s bus\n", busName);
    return true;
}

bool SEService_Reset(void)
{
    if (!g_inited) {
        printf("[SEService] ERROR: Not initialized\n");
        return false;
    }

    printf("[SEService] Reset sequence executed (dummy)\n");

    /* Future extension: implement real RESET toggle or APDU warm reset. */
    return true;
}

void SEService_RegisterNotificationCallback(SeNotificationCb cb, void *ctx)
{
    g_notify_cb = cb;
    g_notify_ctx = ctx;

    if (cb)
        printf("[SEService] Notification callback registered\n");
    else
        printf("[SEService] Notification callback unregistered\n");
}

/* Optional: simulation trigger for testing event flow */
void SEService_SimulateEvent(void)
{
    if (g_notify_cb && g_inited) {
        printf("[SEService] Simulating SE event callback\n");
        g_notify_cb(NULL, g_notify_ctx);
    }
}

bool SEService_IsReady(void)
{
    return g_inited;
}
