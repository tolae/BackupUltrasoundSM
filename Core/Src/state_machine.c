#include "state_machine.h"
#include "tim.h"

state_t previous_state;
state_t current_state;

state_t find_next_state(state_machine_params_t params);
void no_alert_func();
void low_alert_func();
void medium_alert_func();
void high_alert_func();
void critical_alert_func();

void (*state_executions[END_STATE]) (void) =
{
	no_alert_func,
	low_alert_func,
	medium_alert_func,
	high_alert_func,
	critical_alert_func,
};

state_t initialize_state_machine()
{
	current_state = NO_ALERT;
	previous_state = NO_ALERT;
	(*state_executions[NO_ALERT])();
	return current_state;
}

state_t update_state_machine(state_machine_params_t params)
{
	state_t next_state;
	/* Find next state transition and save it */
	next_state = find_next_state(params);
	/* If there was a transition, save the previous state */
	previous_state = next_state != current_state ? current_state : previous_state;
	/* Execute the current state's function */
	(*state_executions[next_state])();
	/* Update the state machine */
	current_state = next_state;
	return current_state;
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

state_t find_next_state(state_machine_params_t params)
{
	state_t next_state;

	switch (current_state)
	{
	case NO_ALERT:
		if (params.distance < 30)
		{
			next_state = LOW_ALERT;
		}
		else
		{
			next_state = current_state;
		}
		break;
	case LOW_ALERT:
		if (params.distance > 30)
		{
			next_state = NO_ALERT;
		}
		else if (params.distance < 20)
		{
			next_state = MEDIUM_ALERT;
		}
		else
		{
			next_state = current_state;
		}
		break;
	case MEDIUM_ALERT:
		if (params.distance > 20)
		{
			next_state = LOW_ALERT;
		}
		else if (params.distance < 12)
		{
			next_state = HIGH_ALERT;
		}
		else
		{
			next_state = current_state;
		}
		break;
	case HIGH_ALERT:
		if (params.distance > 12)
		{
			next_state = MEDIUM_ALERT;
		}
		else if (params.distance < 8)
		{
			next_state = CRITICAL_ALERT;
		}
		else
		{
			next_state = current_state;
		}
		break;
	case CRITICAL_ALERT:
		if (params.distance > 8)
		{
			HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
			next_state = HIGH_ALERT;
		}
		else
		{
			HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);
			next_state = current_state;
		}
		break;
	default:
		/* Error State */
		while (1);
		break;
	}

	return next_state;
}
