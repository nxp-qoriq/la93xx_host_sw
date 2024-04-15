/* SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2021, 2024 NXP
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
#include <string.h>

#include <la9310_modinfo.h>
#include <la9310_wdog_ioctl.h>

#define LA9310_DEV_NAME_PREFIX	"shiva"
#define PCI_DEVICE_PATH "readlink -f /sys/bus/pci/drivers/NXP-LA9310-Driver/"
#define HOST_PCI_PATH "/sys/devices/platform/"
#define DEFAULT_EXTRA_RMMOD_SCRIPT "/home/root/rmmod_rfnm_drivers.sh"
#define xstr(s) str(s)
#define str(s) #s

struct pollfd pfd;

static inline int open_devwdog(int modem_id)
{

	char wdog_dev_name[50];
	int devwdog;

	sprintf(wdog_dev_name, "/dev/%s%s%d",
		LA9310_DEV_NAME_PREFIX, LA9310_WDOG_DEVNAME_PREFIX, modem_id);
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


int libwdog_set_host_state(struct wdog *wdog_t, char *host_pci_status, char value)
{
	int fd;
	char sys_name[160] = {'\0'};
	int ret = MODEM_WDOG_OK;

	sprintf(sys_name, "echo %c > %s\n", value, host_pci_status);
	system(sys_name);
	sleep(1);
	return 0;
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

void get_pci_device_id(int modem_id, char *pci_driver_path) {
	char dev_name[32];
	int fd, ret = 0;
	modinfo_t mil = {0};
	modinfo_t *mi = &mil;
	sprintf(dev_name, "/dev/%s%d", LA9310_DEV_NAME_PREFIX,modem_id);
	fd = open(dev_name, O_RDONLY);
	if (-1 == fd) {
		perror("dev open failed:");
		return;
	}
	ret = ioctl(fd, IOCTL_LA93XX_MODINFO_GET, (modinfo_t *) &mil);
	if (ret < 0) {
		printf("IOCTL_LA93XX_MODINFO_GET failed.\n");
		close(fd);
		return;
	}

	sprintf(pci_driver_path , "%s%s", PCI_DEVICE_PATH, mi->pci_addr);

	close(fd);
	return;
}

void get_pci_controller_path(char *host_pci_status, char *pci_driver_path) {
	FILE *command = popen(pci_driver_path, "r");
	char pci_device_path[256];
	int i = 0, len = 0;

	fgets(pci_device_path, sizeof(pci_device_path), command);

	len += snprintf((host_pci_status + len), sizeof(HOST_PCI_PATH), "%s", HOST_PCI_PATH);

	char *buf = strtok(pci_device_path, "/");
	while (buf != NULL) {
		if (i >= 3) {
			len += sprintf((host_pci_status + len), "%s/", buf);
		}
		if (i == 4) {
			len += sprintf((host_pci_status + len), "pcie_dis");
			break;
		}
		buf = strtok(NULL, "/");
		i++;
	}
	host_pci_status[len] = '\0';
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

int libwdog_reinit_modem_rfnm(struct wdog *wdog_t, uint32_t timeout)
{
	int ret = MODEM_WDOG_OK;
	char host_pci_status[64], pci_driver_path[128], rmmod_script[64];

	get_pci_device_id(wdog_t->wdogid, pci_driver_path);
	get_pci_controller_path(host_pci_status, pci_driver_path);

	strcpy(rmmod_script,xstr(EXTRA_RMMOD_SCRIPT));

	if (rmmod_script == NULL)
		system(DEFAULT_EXTRA_RMMOD_SCRIPT);
	else
		system(rmmod_script);
	/* Remove device from pci subsystem */
	libwdog_remove_modem(wdog_t);
	sleep(1);

	ret = libwdog_set_host_state(wdog_t, host_pci_status, '1');
	if (ret < 0) {
		printf("Modem_wdog: host reset failed\n");
		goto err;
	}

	/* Give reset pulse on MODEM_HRESET */
	ret = libwdog_reset_modem(wdog_t);
	if (ret < 0) {
		printf("Modem_wdog: modem reset failed\n");
		goto err;
	}
	sleep(1);

	ret = libwdog_set_host_state(wdog_t, host_pci_status, '0');
	if (ret < 0) {
		printf("Modem_wdog: host reset failed\n");
		goto err;
	}

	/* Wait for modem to finish boot */
	ret = libwdog_rescan_modem_blocking(wdog_t, timeout);
	if (ret < 0) {
		printf("Modem_wdog: modem rescan fail\n");
		goto err;
	}
err:
	return ret;
}
