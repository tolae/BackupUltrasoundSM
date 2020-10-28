#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "main.h"

#define HYSTERESIS 2 // cm

typedef enum
{
	EQUAL,
	LESS_THAN,
	GREATER_THAN,
	LT_EQUALS,
	GT_EQUALS,
	NOT_EQUALS,
	EMPTY
} transition_type_t;

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
	uint32_t threshold;
	transition_type_t type;
	state_t next_state;
} transition_t;

typedef struct
{
	uint32_t distance;
} state_machine_params_t;

typedef struct
{
	state_t state;
	void (*state_execution) (void);
	transition_t transitions[];
} state_machine_states_t;

state_t initialize_state_machine();
state_t update_state_machine(state_machine_params_t params);

#endif
