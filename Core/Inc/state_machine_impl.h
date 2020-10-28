#ifndef _STATE_MACHINE_IMPL_H
#define _STATE_MACHINE_IMPL_H

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

state_machine_states_t* my_states[END_STATE] =
{
	&no_alert_state,
	&low_alert_state,
	&medium_alert_state,
	&high_alert_state,
	&critical_alert_state
};

state_machine_config_t my_state_machine_config =
{
	.hysteresis = 2,
	.state_machine = my_states,
};

#endif
