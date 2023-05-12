// SPDX-License-Identifier: GPL

#include <linux/errno.h>
#include "logic.h"

int switch_mode(struct logic_state *state, struct logic_mode *mode)
{
	if (state->switching)
		return -EPERM;	// Already switching

	if (state->mode == mode)
		return -EINVAL;	// Skipping switch to the same mode

	state->switching = true;
	state->mode = mode;

	return 0;
}

int next_mode(struct logic_state *state)
{
	state->current_mode += 1;

	if (state->current_mode >= state->mode_count - state->hidden_modes)
		state->current_mode = 0;

	return state->current_mode;
}

int process_state(struct logic_state *state)
{
	if (!state || !state->mode)
		return -EFAULT;

	if (state->switching) {
		state->mode->prepare(state->mode);
		state->switching = false;

		return 0;
	}
	state->mode->cycle(state->mode);

	return 0;
}
