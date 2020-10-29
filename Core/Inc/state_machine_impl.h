#ifndef _STATE_MACHINE_IMPL_H
#define _STATE_MACHINE_IMPL_H

#include "main.h"
#include "state_machine.h"

/* State Machine States */
typedef enum
{
	NO_ALERT = 0,
	LOW_ALERT,
	MEDIUM_ALERT,
	HIGH_ALERT,
	CRITICAL_ALERT,
	END_STATE
} state_t;

/* State Machine Functions */
void no_alert_func();
void low_alert_func();
void medium_alert_func();
void high_alert_func();
void critical_alert_func();
void critical_alert_trans_out_func();
void critical_alert_trans_self_func();

/* States */
/* Self transitions are implied via failure to transition */
state_machine_states_t no_alert_state =
{
	.state = NO_ALERT,
	.state_execution = no_alert_func,
	.transitions =
	{
		{30, LT_EQUALS, LOW_ALERT, NO_TRANS_FUNC},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t low_alert_state =
{
	.state = LOW_ALERT,
	.state_execution = low_alert_func,
	.transitions =
	{
		{30, GREATER_THAN, NO_ALERT, NO_TRANS_FUNC},
		{20, LT_EQUALS, MEDIUM_ALERT, NO_TRANS_FUNC},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t medium_alert_state =
{
	.state = MEDIUM_ALERT,
	.state_execution = medium_alert_func,
	.transitions =
	{
		{20, GREATER_THAN, LOW_ALERT, NO_TRANS_FUNC},
		{12, LT_EQUALS, HIGH_ALERT, NO_TRANS_FUNC},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t high_alert_state =
{
	.state = HIGH_ALERT,
	.state_execution = high_alert_func,
	.transitions =
	{
		{12, GREATER_THAN, MEDIUM_ALERT, NO_TRANS_FUNC},
		{8, LT_EQUALS, CRITICAL_ALERT, NO_TRANS_FUNC},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

state_machine_states_t critical_alert_state =
{
	.state = CRITICAL_ALERT,
	.state_execution = critical_alert_func,
	.transitions =
	{
		{8, GREATER_THAN, HIGH_ALERT, critical_alert_trans_out_func},
		{8, LT_EQUALS, CRITICAL_ALERT, critical_alert_trans_self_func},
		STATE_MACHINE_TRANSITION_TERMINATOR_DECL
	}
};

/* State Machine State Pointer-Array */
state_machine_states_t* my_states[END_STATE] =
{
	&no_alert_state,
	&low_alert_state,
	&medium_alert_state,
	&high_alert_state,
	&critical_alert_state
};

/* Final State Machine Configuration */
state_machine_config_t my_state_machine_config =
{
	.hysteresis = 2,
	.state_machine = my_states,
};

/* State Machine Function Implementation */
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

void critical_alert_trans_out_func()
{
	HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_2);
}

void critical_alert_trans_self_func()
{
	HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_2);
}

/* Used to handle the flashing portion of the critical alert state */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
	HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
}

#endif
