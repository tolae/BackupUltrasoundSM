#include "state_machine.h"
#include "tim.h"

/* Constants */
const state_machine_transition_t STATE_MACHINE_TRANSITION_TERMINATOR =
	STATE_MACHINE_TRANSITION_TERMINATOR_DECL;

/* Private Structs */

/** Internal transition type
 *
 * Packs together the next state and its triggering transition.
 */
typedef struct
{
	state_machine_state_t* next_state;
	state_machine_transition_t triggering_transition;
} transition_t;

/** Internal hysteresis configuration type
 *
 * Contains information about how to handle the hysteresis.
 */
typedef struct
{
	uint32_t hysteresis;
	state_machine_state_enum_t returning_state;
	state_machine_transition_t triggered_transition;
} state_machine_hysteresis_config_t;

/* Private Variables */
state_machine_state_t* previous_state = NULL;
state_machine_state_t* current_state = NULL;
state_machine_hysteresis_config_t hysteresis_config =
{
	.hysteresis = HYSTERESIS_DISABLED,
	.returning_state = INVALID_STATE,
	.triggered_transition = STATE_MACHINE_TRANSITION_TERMINATOR_DECL,
};

/* Primary object for this state machine */
state_machine_state_t** state_machine = NULL;

/* Private Functions */

/** Finds the next state based on the given parameters.
 *
 * @params params The update parameters.
 * @returns A transition if required. Otherwise the current state and terminator.
 */
transition_t find_next_state(state_machine_params_t params);

/** Updates the hysteresis object.
 *
 * The hysteresis is designed to only be active when within the transition
 * bounds AFTER a transition. Once it leaves those bounds AFTER a transition,
 * the hysteresis turns off and no longer affects the transition thresholds.
 *
 * This function will return immediately if the hysteresis is disabled.
 *
 * @params next_state The next state transition that was found.
 * @params params The update parameters.
 */
void update_hysteresis_thresholds(
	transition_t next_state,
	state_machine_params_t params
);

/** Checks to see if a transition is necessary.
 *
 * @param transition The transition to verify.
 * @param params The update parameters to compare against.
 * @returns A boolean based on the transition comparator type.
 */
uint8_t check_transition(
	state_machine_transition_t transition,
	state_machine_params_t params
);

/* Public Function Implementations */

state_machine_state_enum_t initialize_state_machine(state_machine_config_t config)
{
	state_machine = config.state_machine;

	current_state = state_machine[INITIAL_STATE];
	previous_state = state_machine[INITIAL_STATE];
	hysteresis_config.hysteresis = config.hysteresis;
	hysteresis_config.returning_state = INVALID_STATE;
	current_state->state_execution();
	return current_state->state;
}

state_machine_state_enum_t update_state_machine(state_machine_params_t params)
{
	if (state_machine == NULL) return INVALID_STATE;

	transition_t transition;

	/* Find next state transition and save it */
	transition = find_next_state(params);
	update_hysteresis_thresholds(transition, params);
	/* Check to see if there was a transition */
	if (transition.next_state->state != current_state->state)
	{
		/* Save the previous state */
		previous_state = current_state;
		/* Activate hysteresis and update parameters */
		hysteresis_config.returning_state = previous_state->state;
		update_hysteresis_thresholds(transition, params);
	}

	/* Execute the transition's function */
	transition.triggering_transition.transition_func();
	/* Execute the next state's function */
	transition.next_state->state_execution();
	/* Update the state machine */
	current_state = transition.next_state;
	return current_state->state;
}

/* Private Function Implementations */

transition_t find_next_state(state_machine_params_t params)
{
	transition_t state_machine_update;
	state_machine_transition_t transition;
	int i = 0;

	/* Check every transition for the current state */
	do
	{
		transition = state_machine[current_state->state]->transitions[i++];
		if (check_transition(transition, params))
		{
			state_machine_update.next_state = state_machine[transition.next_state];
			state_machine_update.triggering_transition = transition;
			return state_machine_update;
		}
	} while (transition.type != EMPTY);

	/* If no transition detected, return the current one. */
	state_machine_update.next_state = current_state;
	state_machine_update.triggering_transition = STATE_MACHINE_TRANSITION_TERMINATOR;
	return state_machine_update;
}

void update_hysteresis_thresholds(
	transition_t next_state,
	state_machine_params_t params
)
{
	int32_t threshold;

	/* Check if hysteresis is active */
	if (hysteresis_config.returning_state == INVALID_STATE &&
			hysteresis_config.hysteresis != HYSTERESIS_DISABLED)
		return;

	/* Check if there was a transition */
	if (next_state.triggering_transition.type != EMPTY)
	{
		/* Save the transition and the corresponding state */
		hysteresis_config.returning_state = previous_state->state;
		hysteresis_config.triggered_transition = next_state.triggering_transition;
	}

	/* Check if outside of threshold */
	switch(hysteresis_config.triggered_transition.type)
	{
	case LESS_THAN:
	case LT_EQUALS:
		/* Subtract hysteresis if approaching from the right */
		threshold = hysteresis_config
			.triggered_transition
			.threshold.distance - hysteresis_config.hysteresis;
		if (params.distance <= threshold)
		{
			hysteresis_config.returning_state = INVALID_STATE;
		}
		break;
	case GREATER_THAN:
	case GT_EQUALS:
		/* Add hysteresis if approaching from the left */
		threshold = hysteresis_config
			.triggered_transition
			.threshold.distance + hysteresis_config.hysteresis;
		if (params.distance >= threshold)
		{
			hysteresis_config.returning_state = INVALID_STATE;
		}
		break;
	default:
		/* Equals and Not Equals should not have a hysteresis */
		hysteresis_config.returning_state = INVALID_STATE;
		break;
	}
}

uint8_t check_transition(
	state_machine_transition_t transition,
	state_machine_params_t params
)
{
	/* Check if the hysteresis is active and apply it */
	uint8_t is_hysteresis_active = (hysteresis_config.returning_state == transition.next_state);
	uint32_t hysteresis_adjustment = (is_hysteresis_active ? hysteresis_config.hysteresis : HYSTERESIS_DISABLED);
	switch(transition.type)
	{
	case EQUAL:
		return params.distance == transition.threshold.distance;
	case LESS_THAN:
		return params.distance < transition.threshold.distance - hysteresis_adjustment;
	case GREATER_THAN:
		return params.distance > transition.threshold.distance + hysteresis_adjustment;
	case LT_EQUALS:
		return params.distance <= transition.threshold.distance - hysteresis_adjustment;
	case GT_EQUALS:
		return params.distance >= transition.threshold.distance + hysteresis_adjustment;
	case NOT_EQUALS:
		return params.distance != transition.threshold.distance;
	default:
		return 0;
	}
}

void STATE_MACHINE_NO_FUNC() {}
