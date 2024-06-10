#include <string.h>
#include <time.h>
#include <zephyr/device.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/device_runtime.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int telit_modem_operator_init(void);

int main(void)
{
	LOG_INF("platform.");

	telit_modem_operator_init();

	while (1)
	{
		k_sleep(K_SECONDS(1));
	}
	

	return 0;
}
