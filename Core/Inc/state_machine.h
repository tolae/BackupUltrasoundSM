#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "main.h"

typedef struct
{
	uint32_t distance;
} state_machine_params_t;

typedef enum
{
	NO_ALERT = 0,
	LOW_ALERT,
	MEDIUM_ALERT,
	HIGH_ALERT,
	CRITICAL_ALERT,
	END_STATE
} state_t;

typedef struct
{

};

state_t initialize_state_machine();
state_t update_state_machine(state_machine_params_t params);

#endif
