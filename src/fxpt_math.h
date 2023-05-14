/* SPDX-License-Identifier: GPL */

#ifndef _FXPT_MATH_H_
#define _FXPT_MATH_H_

#include <linux/types.h>

#define FXPT_PI	0x8000

#ifndef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
#endif

extern int16_t fxpt_atan2(const int32_t y, const int32_t x);

#endif /* _FXPT_MATH_H_ */
