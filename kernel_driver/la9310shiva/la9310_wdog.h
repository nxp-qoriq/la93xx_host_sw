/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2021 NXP
 */
#ifndef __LA9310_WDOG__
#define __LA9310_WDOG__

#include "la9310_wdog_ioctl.h"

#define MAX_WDOG_DEV            1

enum la9310_wdog_id {
	WDOG_ID_0 = 0,
	WDOG_ID_MAX,
};

int get_wdog_msi(int wdog_id);
/**
 * @brief Register interrupt for modem watchdog event
 * @param[in] modem_id (0...MAX_WATCHDOG_INSTANCE)
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_register(struct wdog *wdog_t, int modem_id);

/**
 * @brief wait for modem watchdog event
 * @param[in] dev_wdog_handle watchdog device fd
 * @param[in] buf buffer to store modem event id
 * @param[in] count byte counte to be read
 * @return On success 0. On failure negative error number
 */
int libwdog_readwait(int dev_wdog_handle, void *buf, int count);

/**
 * @brief Initiate modem reset by toggling HW Pin
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_reset_modem(struct wdog *wdog_t);

/**
 * @brief Unregister modem watchdog interrupt and free IRQ
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_deregister(struct wdog *wdog_t);

/**
 * @brief Get modem current status ready/not ready in wdog->wdog_modem_status
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_get_modem_status(struct wdog *wdog_t);

/**
 * @brief Remove modem from linux pcie subsystem
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_remove_modem(struct wdog *wdog_t);

/**
 * @brief Rescan linux pci bus to detect new modem device and renumerate
 * @return On success 0. On failure negative error number
 */
int libwdog_rescan_modem(void);

/**
 * @brief Rescan linux pci bus to detect new modem device
 * and wait till modem becomes ready. wdog->wdog_modem_status
 * will contian modem status
 * @param[in] wdog_t reference to struct wdog
 * @param[in] timeout timeout in seconds
 * @return On success 0. On failure negative error number
 */
int libwdog_rescan_modem_blocking(struct wdog *wdog_t, uint32_t timeout);

/**
 * @brief Reinitalize modem. Performs 3 operations
 * 1. Remove modem from linux pci subsystem
 * 2. Hard resets modem
 * 3. Rescan the pci bus until modem becomes ready
 * @param[in] wdog_t reference to struct wdog
 * @param[in] timeout timeout in seconds
 * @return On success 0. On failure negative error number
 */
int libwdog_reinit_modem(struct wdog *wdog_t, uint32_t timeout);

/** @} */
#endif /* __LA9310_WDOG__ */

