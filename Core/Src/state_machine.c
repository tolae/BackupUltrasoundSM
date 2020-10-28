#include "state_machine.h"
#include "tim.h"

/* Private Defines */
#define STATE_MACHINE_TRANSITION_TERMINATOR_DECL {END_STATE, EMPTY, 0}

/* Constants */
const transition_t STATE_MACHINE_TRANSITION_TERMINATOR = STATE_MACHINE_TRANSITION_TERMINATOR_DECL;

/* Private Structs */
typedef struct
{
	state_machine_states_t* next_state;
	transition_t triggering_transition;
} state_machine_transition_t;

/* State Machine Functions */
void no_alert_func();
void low_alert_func();
void medium_alert_func();
void high_alert_func();
void critical_alert_func();

/* States */
/* Self transitions are implied via failure to transition */
state_machine_states_t no_alert_state =
{
	.state = NO_ALERT,
	.state_execution = no_alert_func,
	.transitions =
	{
		{30, LT_EQUALS, LOW_ALERT},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t low_alert_state =
{
	.state = LOW_ALERT,
	.state_execution = low_alert_func,
	.transitions =
	{
		{30, GREATER_THAN, NO_ALERT},
		{20, LT_EQUALS, MEDIUM_ALERT},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t medium_alert_state =
{
	.state = MEDIUM_ALERT,
	.state_execution = medium_alert_func,
	.transitions =
	{
		{20, GREATER_THAN, LOW_ALERT},
		{12, LT_EQUALS, HIGH_ALERT},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t high_alert_state =
{
	.state = HIGH_ALERT,
	.state_execution = high_alert_func,
	.transitions =
	{
		{12, GREATER_THAN, MEDIUM_ALERT},
		{8, LT_EQUALS, CRITICAL_ALERT},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t critical_alert_state =
{
	.state = CRITICAL_ALERT,
	.state_execution = critical_alert_func,
	.transitions =
	{
		{8, GREATER_THAN, HIGH_ALERT},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

/* Private Variables */
state_machine_states_t* previous_state;
state_machine_states_t* current_state;
state_machine_states_t hysteresis =
{
	.state = END_STATE,
	.state_execution = (void *)NULL,
	.transitions =
	{
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};
state_machine_states_t* my_states[END_STATE] =
{
	&no_alert_state,
	&low_alert_state,
	&medium_alert_state,
	&high_alert_state,
	&critical_alert_state,
};

/* Private Functions */
state_machine_transition_t find_next_state(state_machine_params_t params);
void update_hysteresis_thresholds(state_machine_transition_t next_state, state_machine_params_t params);
uint32_t check_transition(transition_t transition, state_machine_params_t params);

state_t initialize_state_machine()
{
	current_state = &no_alert_state;
	previous_state = &no_alert_state;
	hysteresis.state = END_STATE;
	current_state->state_execution();
	return current_state->state;
}

state_t update_state_machine(state_machine_params_t params)
{
	state_machine_transition_t transition;

	/* Find next state transition and save it */
	transition = find_next_state(params);
	update_hysteresis_thresholds(transition, params);
	/* Check to see if there was a transition */
	if (transition.next_state->state != current_state->state)
	{
		/* Save the previous state */
		previous_state = current_state;
		/* Activate hysteresis and update parameters */
		hysteresis.state = previous_state->state;
		update_hysteresis_thresholds(transition, params);
	}

	/* Execute the next state's function */
	transition.next_state->state_execution();
	/* Update the state machine */
	current_state = transition.next_state;
	return current_state->state;
}

state_machine_transition_t find_next_state(state_machine_params_t params)
{
	state_machine_transition_t state_machine_update;
	transition_t transition;
	int i = 0;

	do
	{
		transition = my_states[current_state->state]->transitions[i++];
		if (check_transition(transition, params))
		{
			state_machine_update.next_state = my_states[transition.next_state];
			state_machine_update.triggering_transition = transition;
			return state_machine_update;
		}
	} while (transition.type != EMPTY);

	state_machine_update.next_state = current_state;
	state_machine_update.triggering_transition = STATE_MACHINE_TRANSITION_TERMINATOR;
	return state_machine_update;
}

void update_hysteresis_thresholds(state_machine_transition_t next_state, state_machine_params_t params)
{
	/* Check if hysteresis is active */
	if (hysteresis.state == END_STATE) return;

	/* Check if there was a transition */
	if (next_state.triggering_transition.type != EMPTY)
	{
		/* Save the transition and the corresponding state */
		hysteresis.state = previous_state->state;
		hysteresis.transitions[0] = next_state.triggering_transition;
	}

	/* Check if outside of threshold */
	switch(hysteresis.transitions[0].type)
	{
	case LESS_THAN:
	case LT_EQUALS:
		if (params.distance <= hysteresis.transitions[0].threshold - HYSTERESIS)
		{
			hysteresis.state = END_STATE;
		}
		break;
	case GREATER_THAN:
	case GT_EQUALS:
		if (params.distance >= hysteresis.transitions[0].threshold + HYSTERESIS)
		{
			hysteresis.state = END_STATE;
		}
		break;
	default:
		/* Equals and Not Equals should not have a hysteresis */
		hysteresis.state = END_STATE;
		break;
	}
}

uint32_t check_transition(transition_t transition, state_machine_params_t params)
{
	switch(transition.type)
	{
	case EQUAL:
		return params.distance == transition.threshold;
	case LESS_THAN:
		return params.distance < transition.threshold - (hysteresis.state == transition.next_state) ? HYSTERESIS : 0;
	case GREATER_THAN:
		return params.distance > transition.threshold + (hysteresis.state == transition.next_state) ? HYSTERESIS : 0;
	case LT_EQUALS:
		return params.distance <= transition.threshold - (hysteresis.state == transition.next_state) ? HYSTERESIS : 0;
	case GT_EQUALS:
		return params.distance >= transition.threshold + (hysteresis.state == transition.next_state) ? HYSTERESIS : 0;
	case NOT_EQUALS:
		return params.distance != transition.threshold;
	default:
		return 0;
	}
}

void no_alert_func()
{
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
}

void low_alert_func()
{
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
}

void medium_alert_func()
{
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
}

void high_alert_func()
{
	HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
}

void critical_alert_func()
{
	/* The red flashing portion is handled inside TIM2 timer callback. */
	HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_RESET);
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
}
