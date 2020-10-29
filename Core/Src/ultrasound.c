#include "ultrasound.h"
#include "tim.h"

uint32_t last_read_us;
uint32_t timer_triggers_us[2];

void enable_ultrasound()
{
	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, timer_triggers_us, 2);
	HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
}

void disable_ultrasound()
{
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_2);
}

float get_read_cm()
{
	return get_read_us() / 58.0f;
}

uint32_t get_read_us()
{
	return last_read_us;
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	/* Only htim2 is configured for callbacks */
	last_read_us = timer_triggers_us[1] - timer_triggers_us[0];
}

void HAL_TIM_IC_CaptureHalfCpltCallback(TIM_HandleTypeDef *htim)
{
//	htim->Instance->CNT = 0;
}
