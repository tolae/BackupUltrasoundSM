#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "main.h"

#define HYSTERESIS 2 // cm
#define STATE_MACHINE_TRANSITION_TERMINATOR_DECL {-1, EMPTY, 0}

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

typedef struct
{
	int32_t threshold;
	transition_type_t type;
	int32_t next_state;
} transition_t;

typedef struct
{
	int32_t distance;
} state_machine_params_t;

typedef struct
{
	int32_t state;
	void (*state_execution) (void);
	transition_t transitions[];
} state_machine_states_t;

typedef struct
{
	uint32_t hysteresis;
	state_machine_states_t** state_machine;
} state_machine_config_t;

int32_t initialize_state_machine(state_machine_config_t config);
int32_t update_state_machine(state_machine_params_t params);

#endif
