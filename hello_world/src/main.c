#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "posix_board_if.h"

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
#if 1
	posix_exit(0);
#endif	
	return 0;
}