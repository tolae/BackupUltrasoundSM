#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

/* Includes */
#include "main.h"

/* Defines */

/* Value for disabled hysteresis */
#define HYSTERESIS_DISABLED 0

/* The initial state for any state machine should be 0. */
#define INITIAL_STATE 0

/** An invalid state for a state machine should be negative.
 *
 * This does NOT mean error states should be negative. A negative
 * state indicates an internal error to the state machine process
 * or a null state.
 */
#define INVALID_STATE -1

#define STATE_MACHINE_TRANSITION_TERMINATOR_DECL \
	{ {INVALID_STATE}, EMPTY, 0, STATE_MACHINE_NO_FUNC}

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
 * Defines that a state machine state should be an integer. Used since an
 * enum cannot be typedef'd without an implementation nor check if a typedef
 * has been made or not at compile time.
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
 *
 * @params config A list of states and a hysteresis value.
 * @returns The initial state for the state machine.
 */
state_machine_state_enum_t initialize_state_machine(state_machine_config_t config);

/** Updates the state machine with the new parameters.
 *
 * This should be called as frequently as desired. It will handle state
 * transitions as well as state calls.
 *
 * @params params The state machine parameters to update with.
 * @returns The updated state machine state or -1 if uninitialized.
 */
state_machine_state_enum_t update_state_machine(state_machine_params_t params);

#endif
