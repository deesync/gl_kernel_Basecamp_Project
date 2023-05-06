/* SPDX-License-Identifier: GPL */

#ifndef __LOGIC_MODULE_H__
#define __LOGIC_MODULE_H__

#define SM_TXT_OFFSET 16

struct logic_mode {
	int (*prepare)(struct logic_mode *);
	int (*cycle)(struct logic_mode *);
};

#endif /*__LOGIC_MODULE_H__ */
