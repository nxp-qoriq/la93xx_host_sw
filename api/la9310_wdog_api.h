/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2021-2024 NXP
 */

#ifndef __LA9310_WDOG_API__
#define __LA9310_WDOG_API__

#include "la9310_wdog_ioctl.h"

/** \addtogroup  HOST_LIBWDOG_API
 *  @{
 */

/**
 * @brief Register interrupt for modem watchdog event
 * @param[in] wdog_t reference to struct wdog
 * @param[in] modem_id watchdog modem id
 * @return On success 0. On failure negative error number
 */
int libwdog_open(struct wdog *wdog_t, int modem_id);

/**
 * @brief Register interrupt for modem watchdog event
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_register(struct wdog *wdog_t);

/**
 * @brief wait for modem watchdog event
 * @param[in] dev_wdog_handle watchdog device fd
 * @return On success 0. On failure negative error number
 */
int libwdog_wait(int dev_wdog_handle);

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
 * @brief Close watchdog device
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_close(struct wdog *wdog_t);
/**
 * @brief Get modem current status ready/not ready in wdog->wdog_modem_status
 * @param[in] wdog_t reference to struct wdog
 * @return On success 0. On failure negative error number
 */
int libwdog_get_modem_status(struct wdog *wdog_t);

/**
 * @brief Set host pci state. Used to reset host pci.
 * @param[in] wdog_t reference to struct wdog
 * 	      value  0/1 (shutdown/power up) host pci
 * @return On success 0. On failure negative error number
 */
int libwdog_set_host_state(struct wdog *wdog_t, char value);

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

/**
 * @brief Reinitalize modem. Performs 3 operations
 * 1. Remove all rfnm modules
 * 2. Remove modem from linux pci subsystem
 * 3. Shutdown the host pci controller
 * 4. Hard resets modem
 * 5. Power up the host pci controller
 * 6. Rescan the pci bus until modem becomes ready
 * @param[in] wdog_t reference to struct wdog
 * @param[in] timeout timeout in seconds
 * @return On success 0. On failure negative error number
 */
int libwdog_reinit_modem_rfnm(struct wdog *wdog_t, uint32_t timeout);
/** @} */
#endif /* __LA9310_WDOG__ */

