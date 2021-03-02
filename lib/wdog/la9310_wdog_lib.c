/*
 *
 * Copyright 2021 NXP
 *
 * Author:Pramod Kumar <pramod.kumar_1@nxp.com>
 *
 * This software is available to you under the BSD-3-Clause
 * license mentioned below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>

#include <la9310_wdog_ioctl.h>

#define LA9310_WDOG_DEVNAME_PREFIX "la9310wdog"
#define LA9310_WDOG_DEV0_NAME       "dev0"

struct pollfd pfd;

static inline int open_devwdog(int modem_id)
{

	char wdog_dev_name[50];
	int devwdog;

	sprintf(wdog_dev_name, "/dev/%s%d", LA9310_WDOG_DEVNAME_PREFIX, modem_id);
	devwdog = open(wdog_dev_name, O_RDWR | O_NONBLOCK);
	if (devwdog < 0)
		return devwdog;

	pfd.fd = devwdog;
	pfd.events = (POLLIN | POLLRDNORM | POLLOUT | POLLWRNORM);

	return devwdog;
}

int libwdog_open(struct wdog *wdog_t, int modem_id)
{
	int32_t ret = MODEM_WDOG_OK;

	wdog_t->wdogid = modem_id;
	/* Register Watchdog */
	wdog_t->dev_wdog_handle = open_devwdog(modem_id);
	if (wdog_t->dev_wdog_handle < 0)
		ret = -MODEM_WDOG_OPEN_FAIL;

	return ret;
}

int libwdog_register(struct wdog *wdog_t)
{
	return ioctl(wdog_t->dev_wdog_handle,
		     IOCTL_LA9310_MODEM_WDOG_REGISTER, wdog_t);
}

static inline int close_devwdog(int dev_wdog_handle)
{
	return close(dev_wdog_handle);
}

static int get_pci_domain_nr(struct wdog *wdog_t)
{
	/* get pci slot (domain_nr) */
	return ioctl(wdog_t->dev_wdog_handle,
		     IOCTL_LA9310_MODEM_WDOG_GET_DOMAIN, wdog_t);
}

int libwdog_deregister(struct wdog *wdog_t)
{
	/* Free IRQs */
	return ioctl(wdog_t->dev_wdog_handle,
		     IOCTL_LA9310_MODEM_WDOG_DEREGISTER, wdog_t);
}

int libwdog_close(struct wdog *wdog_t)
{
	return close_devwdog(wdog_t->dev_wdog_handle);
}

int libwdog_wait(int dev_wdog_handle)
{
	int32_t ret = MODEM_WDOG_OK;

	ret = poll(&pfd, 1, -1);

	if (ret < 0)
		perror("WDOG poll fail");
	else
		printf("WDOG trigger event received..\n");

	return ret;
}

int libwdog_reset_modem(struct wdog *wdog_t)
{
	return ioctl(wdog_t->dev_wdog_handle, IOCTL_LA9310_MODEM_WDOG_RESET, wdog_t);
}

int libwdog_get_modem_status(struct wdog *wdog_t)
{
	return ioctl(wdog_t->dev_wdog_handle,
			IOCTL_LA9310_MODEM_WDOG_GET_STATUS,
			wdog_t);
}

int libwdog_remove_modem(struct wdog *wdog_t)
{
	int fd;
	char sys_name[128] = {'\0'};
	char value = '1';
	int ret = MODEM_WDOG_OK;

	ret = get_pci_domain_nr(wdog_t);
	if (ret < 0)
		goto err;

	if (wdog_t->domain_nr < 0)
		goto err;

	snprintf(sys_name, sizeof(sys_name), "%s/%04x:00:00.0/remove",
					MODEM_PCI_DEVICE_PATH,
					wdog_t->domain_nr);
	fd = open(sys_name, O_WRONLY);
	if (fd < 0) {
		ret = -MODEM_WDOG_OPEN_FAIL;
		goto err;
	}
	ret = write(fd, &value, 1);
	if (ret < 0) {
		close(fd);
		goto err;
	}
	close(fd);
err:
	return ret;
}

int libwdog_rescan_modem(void)
{
	int fd;
	char value = '1';
	int ret = MODEM_WDOG_OK;

	fd = open(MODEM_PCI_RESCAN_FILE, O_WRONLY);
	if (fd < 0) {
		ret = -MODEM_WDOG_OPEN_FAIL;
		goto err;
	}
	ret = write(fd, &value, 1);
	if (ret <= 0) {
		close(fd);
		ret = -MODEM_WDOG_WRITE_FAIL;
		goto err;
	}
	close(fd);
err:
	return ret;
}

int libwdog_rescan_modem_blocking(struct wdog *wdog_t, uint32_t timeout)
{
	int ret = MODEM_WDOG_OK;

	if (timeout > 0) {
		while (timeout--) {
			printf("Scanning %d\n", timeout);
			/* rescan pci */
			ret = libwdog_rescan_modem();
			if (ret < 0)
				goto err;

			/* check if modem alreay in ready state */
			ret = libwdog_get_modem_status(wdog_t);
			if (ret < 0)
				goto err;
			if (wdog_t->wdog_modem_status == WDOG_MODEM_READY)
				break;
			sleep(1);
		}
	} else {
		/* rescan pci */
		ret = libwdog_rescan_modem();
		if (ret < 0)
			goto err;
		sleep(1);
		/* check if modem alreay in ready state */
		ret = libwdog_get_modem_status(wdog_t);
		if (ret < 0)
			goto err;
	}
err:
	return ret;
}

int libwdog_reinit_modem(struct wdog *wdog_t, uint32_t timeout)
{
	int ret = MODEM_WDOG_OK;

	/* Remove device from pci subsystem */
	libwdog_remove_modem(wdog_t);

	/* Give reset pulse on MODEM_HRESET */
	ret = libwdog_reset_modem(wdog_t);
	if (ret < 0) {
		printf("Modem_wdog: modem reset failed\n");
		goto err;
	}
	sleep(1);
	/* Wait for modem to finish boot */
	ret = libwdog_rescan_modem_blocking(wdog_t, timeout);
	if (ret < 0) {
		printf("Modem_wdog: modem rescan fail\n");
		goto err;
	}
err:
	return ret;
}
