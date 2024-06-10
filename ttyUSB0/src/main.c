#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#include <stdio.h>
#include <string.h>

const struct device *uart0 = DEVICE_DT_GET(DT_NODELABEL(uart0));

void send_str(const struct device *uart, char *str)
{
	int msg_len = strlen(str);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart, str[i]);
	}

	printk("Device %s sent: \"%s\"\n", uart->name, str);
}

void recv_str(const struct device *uart, char *str)
{
	char *head = str;
	char c;

	while (!uart_poll_in(uart, &c)) {
		*head++ = c;
	}
	*head = '\0';

	printk("Device %s received: \"%s\"\n", uart->name, str);
}

int main(void)
{
	char recv_buf[64];
	int i = 10;

	while (i--) {
		send_str(uart0, "AT\r\n");
		/* Wait some time for the messages to arrive to the second uart. */
		k_sleep(K_MSEC(100));
		recv_str(uart0, recv_buf);

		k_sleep(K_MSEC(1000));
	}

	return 0;
}