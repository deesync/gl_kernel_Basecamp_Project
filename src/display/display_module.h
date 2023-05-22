/* SPDX-License-Identifier: GPL */

#ifndef __DISPLAY_MODULE_H__
#define __DISPLAY_MODULE_H__

#include "ssd1306.h"
#include "display_fonts.h"

#define MAX_STR_LEN 21

extern int bc_display_clear(void);
extern int bc_display_print(u8 offset, u8 line,
			    const struct display_font_t *font, char *str);

#endif // __DISPLAY_MODULE_H__
