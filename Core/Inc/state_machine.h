#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

/* Includes */
#include "main.h"

/* Defines */
#define STATE_MACHINE_TRANSITION_TERMINATOR_DECL \
	{ {-1}, EMPTY, 0, STATE_MACHINE_NO_FUNC}

/* Typedefs */

/** Transition types
 *
 * Used to define how to compare the transition
 * threshold with the input parameter.
 */
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

/** Transition function type
 *
 * All transitions should not return anything and should accept no arguments.
 */
typedef void (*transition_func_t) (void);

/** State machine function type
 *
 * Useful for when a state has an internal looping function.
 * Called on every update.
 */
typedef void (*state_machine_func_t) (void);

/** State machine state type
 *
 * Defines that a state machine state should be an integer.
 */
typedef int32_t state_machine_state_enum_t;

/** State machine parameter
 *
 * This state machine currently only accepts a
 * single, continuous variable as a parameter.
 */
typedef struct
{
	int32_t distance;
} state_machine_params_t;

/** State machine transition struct
 *
 * Contains information about a specific transition.
 */
typedef struct
{
	state_machine_params_t threshold;
	transition_type_t type;
	state_machine_state_enum_t next_state;
	transition_func_t transition_func;
} state_machine_transition_t;

/** State machine states struct
 *
 * Holds information about the state, its execution function,
 * and the available transitions for that state.
 */
typedef struct
{
	state_machine_state_enum_t state;
	state_machine_func_t state_execution;
	state_machine_transition_t transitions[];
} state_machine_state_t;

/** State machine configuration struct
 *
 * Contains the configuration information
 * required for the state machine to operate.
 */
typedef struct
{
	uint32_t hysteresis;
	state_machine_state_t** state_machine;
} state_machine_config_t;

/* Empty function for transitions and states that have no function call */
void STATE_MACHINE_NO_FUNC();

/* Public Functions */

/** Initializes the state machine with the given configuration.
 *
 * Only after this function is called can the state machine be used.
 */
int32_t initialize_state_machine(state_machine_config_t config);

/** Updates the state machine with the new parameters.
 *
 * This should be called as frequently as desired. It will handle state
 * transitions as well as state calls.
 */
int32_t update_state_machine(state_machine_params_t params);

#endif
