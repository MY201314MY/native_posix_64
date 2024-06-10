#include <string.h>
#include <time.h>
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
LOG_MODULE_REGISTER(operator, LOG_LEVEL_INF);

static void modem_cellular_transparent_handler(struct ring_buf *ring)
{
	uint8_t cnt;
	uint8_t rxbuffer[1];

	while( 1 )
	{
		cnt = (uint8_t)ring_buf_get(ring, rxbuffer, sizeof( rxbuffer ) );
    	if(cnt <= 0)
    	{
      		break;
    	}
    	printk( "%c", rxbuffer[0] );
  	}
}

static void modem_chat_callback_handler(struct modem_chat *chat,
					enum modem_chat_script_result result, void *user_data)
{
	switch (result) {
	case MODEM_CHAT_SCRIPT_RESULT_SUCCESS:
		LOG_DBG("%d", __LINE__);
		break;

	case MODEM_CHAT_SCRIPT_RESULT_ABORT:
		LOG_DBG("%d", __LINE__);
		break;

	case MODEM_CHAT_SCRIPT_RESULT_TIMEOUT:
		LOG_DBG("%d", __LINE__);
		break;
	}
}

static void modem_cellular_chat_on_response(struct modem_chat *chat, char **argv, uint16_t argc, void *user_data)
{
	LOG_WRN("%s", argv[1]);
}

MODEM_CHAT_MATCH_DEFINE(ok_match, "OK", "", NULL);
MODEM_CHAT_MATCHES_DEFINE(abort_matches, MODEM_CHAT_MATCH("ERROR", "", NULL));
MODEM_CHAT_MATCH_DEFINE(response_match, "", "", modem_cellular_chat_on_response);

static struct modem_chat_script_chat mex10g1_chat_script_cmds[] = 
{
	MODEM_CHAT_SCRIPT_CMD_RESP("ATE0", ok_match),

	MODEM_CHAT_SCRIPT_CMD_RESP("AT+SYSFLASH?", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", ok_match),

	MODEM_CHAT_SCRIPT_CMD_RESP("AT+GMR", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", ok_match),

	MODEM_CHAT_SCRIPT_CMD_RESP("AT+SYSTIMESTAMP?", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", ok_match),

	MODEM_CHAT_SCRIPT_CMD_RESP("AT+SYSRAM?", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", ok_match),

	MODEM_CHAT_SCRIPT_CMD_RESP("AT+SYSMSG?", response_match),
	MODEM_CHAT_SCRIPT_CMD_RESP("", ok_match),
};

struct modem_chat_script mex10g1_chat_script = { 
	.name = "telit mex10g1 chat script", 
	.script_chats = mex10g1_chat_script_cmds,
	.script_chats_size = ARRAY_SIZE(mex10g1_chat_script_cmds), 
	.abort_matches = abort_matches, 
	.abort_matches_size = 1, 
	.callback = modem_chat_callback_handler, 
	.timeout = 10, 
};

static struct modem_backend_tty tty_backend;
static struct modem_pipe *tty_pipe = NULL;

struct modem_chat chat;
static uint8_t chat_receive_buf[128];
static uint8_t chat_delimiter[2] = {'\r', '\n'};
static uint8_t chat_filter[2] = {'\r', '\n'};
static uint8_t *chat_argv[32];

K_KERNEL_STACK_DEFINE(tty_stack, 4096);

int telit_modem_operator_init(void)
{
	const struct modem_backend_tty_config backend_tty_config = {
		.tty_path = "/dev/ttyUSB1",
		.stack = tty_stack,
		.stack_size = K_KERNEL_STACK_SIZEOF(tty_stack),
	};

	tty_pipe = modem_backend_tty_init(&tty_backend, &backend_tty_config);
	LOG_DBG("pipe : %p", tty_pipe);

	int ret = modem_pipe_open(tty_pipe);
	if(ret<0)
	{
		LOG_ERR("pipe open failed, ret = %d", ret);
		return -ENXIO;
	}

	const struct modem_chat_config chat_config = {
		.user_data = NULL,
		.receive_buf = chat_receive_buf,
		.receive_buf_size = ARRAY_SIZE(chat_receive_buf),
		.delimiter = chat_delimiter,
		.delimiter_size = ARRAY_SIZE(chat_delimiter),
		.filter = chat_filter,
		.filter_size = ARRAY_SIZE(chat_filter),
		.argv = chat_argv,
		.argv_size = ARRAY_SIZE(chat_argv),
		.unsol_matches = abort_matches,
		.unsol_matches_size = ARRAY_SIZE(abort_matches),
	};

	ret = modem_chat_init(&chat, &chat_config);
	chat.transparent.callback = modem_cellular_transparent_handler;

	ret = modem_chat_attach(&chat, tty_pipe);
	
	LOG_DBG("timeout : %d", mex10g1_chat_script.timeout);
	LOG_DBG("size : %d", mex10g1_chat_script.script_chats_size);
#if 0
	ret = modem_chat_run_script_async(&chat, &mex10g1_chat_script);
	LOG_DBG("L:%d -- ret = %d", __LINE__, ret);
#endif
	
	return 0;
}

int modem_espressif_receive(uint8_t *rxbuffer, size_t size, k_timeout_t timeout)
{
	uint32_t ret = 0;
	size_t offset = 0;
	int64_t backup = k_uptime_get();
 
	while( 1 )
    {
    	while(ring_buf_is_empty(&chat.transparent.ring) == false)
      	{
        	ret = ring_buf_get(&chat.transparent.ring, rxbuffer + offset, 1);
    		offset += 1;

        	if(offset >= size)
        	{
          		return offset;
        	}

			if(strstr(rxbuffer, "\r\nOK\r\n") != NULL || strstr(rxbuffer, "\r\nERROR\r\n") != NULL)
			{
				return offset;
			}
      	}

      	if((k_uptime_get()-backup) > timeout.ticks*10 )
      	{
        	break;
      	}

      	k_sleep(K_MSEC(20));
    }

  	return offset;
}

void basic_set_up()
{
	int ret = modem_chat_run_script_async(&chat, &mex10g1_chat_script);
	LOG_DBG("L:%d -- ret = %d", __LINE__, ret);
}