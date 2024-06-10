/*
 * Copyright (c) 2023, Bjarki Arge Andreasen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>
#include <string.h>
#include <zephyr/shell/shell.h>
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/modem/chat.h>
#include <zephyr/modem/cmux.h>
#include <zephyr/modem/pipe.h>
#include <zephyr/modem/ppp.h>
#include <zephyr/modem/backend/tty.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(espressif, LOG_LEVEL_DBG);

extern struct modem_chat chat;
void basic_set_up();

int modem_espressif_receive(uint8_t *rxbuffer, size_t size, k_timeout_t timeout);

int espressif_basic_set_up(const struct shell *sh, size_t argc, char *argv[])
{
	basic_set_up();

	return 0;
}

int espressif_scan_wifi(const struct shell *sh, size_t argc, char *argv[])
{
	modem_pipe_transmit(chat.pipe, "AT+CWLAP\r\n", strlen("AT+CWLAP\r\n"));

	return 0;
}

int espressif_get_version(const struct shell *sh, size_t argc, char *argv[])
{
	uint8_t rxbuffer[1024]={0};

	if(k_mutex_lock(&chat.mutex, K_NO_WAIT) == 0)
	{
		modem_pipe_transmit(chat.pipe, "AT+GMR\r\n", strlen("AT+GMR\r\n"));
		int ret = modem_espressif_receive(rxbuffer, sizeof(rxbuffer), K_SECONDS(2));
		LOG_HEXDUMP_INF(rxbuffer, ret, "RX");

		k_mutex_unlock(&chat.mutex);
	}
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(espressif_commands,
	SHELL_CMD(version, NULL,
		"get espressif version",
		espressif_get_version),
	SHELL_CMD(basic, NULL,
		"basic set up",
		espressif_basic_set_up),
	SHELL_CMD(scan, NULL,
		"scan Wi-Fi",
		espressif_scan_wifi),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(espressif, &espressif_commands,
		   "example for esp-at", NULL);