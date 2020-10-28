#include "state_machine.h"
#include "tim.h"

/* Constants */
const transition_t STATE_MACHINE_TRANSITION_TERMINATOR = STATE_MACHINE_TRANSITION_TERMINATOR_DECL;

/* Private Structs */
typedef struct
{
	state_machine_states_t* next_state;
	transition_t triggering_transition;
} state_machine_transition_t;

typedef struct
{
	uint32_t hysteresis;
	int32_t returning_state;
	transition_t triggered_transition;
} state_machine_hysteresis_config_t;

/* Private Variables */
state_machine_states_t* previous_state;
state_machine_states_t* current_state;
state_machine_hysteresis_config_t hysteresis_config =
{
	.hysteresis = 0,
	.returning_state = -1,
	.triggered_transition = STATE_MACHINE_TRANSITION_TERMINATOR_DECL,
};
state_machine_states_t** state_machine;

/* Private Functions */
state_machine_transition_t find_next_state(state_machine_params_t params);
void update_hysteresis_thresholds(state_machine_transition_t next_state, state_machine_params_t params);
uint32_t check_transition(transition_t transition, state_machine_params_t params);

int32_t initialize_state_machine(state_machine_config_t config)
{
	state_machine = config.state_machine;

	current_state = state_machine[0];
	previous_state = state_machine[0];
	hysteresis_config.hysteresis = config.hysteresis;
	hysteresis_config.returning_state = -1;
	current_state->state_execution();
	return current_state->state;
}

int32_t update_state_machine(state_machine_params_t params)
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
		hysteresis_config.returning_state = previous_state->state;
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
		transition = state_machine[current_state->state]->transitions[i++];
		if (check_transition(transition, params))
		{
			state_machine_update.next_state = state_machine[transition.next_state];
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
	if (hysteresis_config.returning_state == -1 && hysteresis_config.hysteresis > 0) return;

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
		if (params.distance <= hysteresis_config.triggered_transition.threshold - HYSTERESIS)
		{
			hysteresis_config.returning_state = -1;
		}
		break;
	case GREATER_THAN:
	case GT_EQUALS:
		if (params.distance >= hysteresis_config.triggered_transition.threshold + HYSTERESIS)
		{
			hysteresis_config.returning_state = -1;
		}
		break;
	default:
		/* Equals and Not Equals should not have a hysteresis */
		hysteresis_config.returning_state = -1;
		break;
	}
}

uint32_t check_transition(transition_t transition, state_machine_params_t params)
{
	int something;
	uint32_t hysteresis_is_active = (hysteresis_config.returning_state == transition.next_state);
	switch(transition.type)
	{
	case EQUAL:
		return params.distance == transition.threshold;
	case LESS_THAN:
		return params.distance < transition.threshold - (hysteresis_is_active ? hysteresis_config.hysteresis : 0);
	case GREATER_THAN:
		return params.distance > transition.threshold + (hysteresis_is_active ? hysteresis_config.hysteresis : 0);
	case LT_EQUALS:
		return params.distance <= transition.threshold - (hysteresis_is_active ? hysteresis_config.hysteresis : 0);
	case GT_EQUALS:
		return params.distance >= transition.threshold + (hysteresis_is_active ? hysteresis_config.hysteresis : 0);
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
