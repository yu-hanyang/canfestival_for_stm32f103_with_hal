/*
 * app_led.c
 *
 *  Created on: Jan 21, 2026
 *      Author: Administrator
 */


#include "app_led.h"

void app_led_init(void){
	TURN_OFF_LED;
}

void app_led_task(void)
{
	static app_led_task_statse_t app_led_task_statse = APP_LED_TASK_INIT;
	switch (app_led_task_statse)
	{
		case APP_LED_TASK_INIT:
			app_led_init();
			app_led_task_statse = APP_LED_TASK_RUNNING;
			break;
		case APP_LED_TASK_RUNNING:
			if (s_controller.led.statse == 0)
			{
				TURN_OFF_LED;
			}
			else
			{
				TURN_ON_LED;
			}
			break;
		default:
			app_led_task_statse = APP_LED_TASK_INIT;
			break;
	}
}

