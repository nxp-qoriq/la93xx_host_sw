/* SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 * Copyright 2017, 2021 NXP
 */

#ifndef __LA9310_HOST_V2H_CALLBACK_H__
#define __LA9310_HOST_V2H_CALLBACK_H__

/*Call back function prototype*/
typedef void (*v2h_callback_t)(struct sk_buff *skb_ptr, void *cookie);

/*****************************************************************************
 * @register_v2h_callback
 *
 * Receiver application uses this API to receive a message, if available.
 * IPC copies message from Rx ring to the buffer provided by application
 *
 * callbackfunc   - [IN][M]  Pointer to callback function
 *
 * skb_ptr        - [OUT][M] sk_buff pointer provided by V2H app where
 *			     V2H driver would pass the pointer to get the
 *                           received Rx frame from VSPA
 *
 * Return Value -
 *      Return 0 on success
 *      positive value on failure
 ****************************************************************************/
int register_v2h_callback(const char *name, v2h_callback_t callbackfunc,
		void *cookie);
int unregister_v2h_callback(const char *name);
#endif /* __LA9310_HOST_V2H_CALLBACK_H__ */
