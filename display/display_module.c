// SPDX-License-Identifier: GPL

#include <linux/module.h>

#define MODULE_NAME "Display"
#define MP MODULE_NAME ": "	/* Log Message Prefix */


int bc_display_data(char *str)
{
	pr_info(MP "%s", str);

	return 0;
}
EXPORT_SYMBOL(bc_display_data);

static int __init display_mod_init(void)
{
	pr_info(MP "module initialized\n");
	return 0;
}

static void __exit display_mod_exit(void)
{
	pr_info(MP "module exited\n");
}

module_init(display_mod_init);
module_exit(display_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Bootcamp Project Display Module");
MODULE_AUTHOR("");
