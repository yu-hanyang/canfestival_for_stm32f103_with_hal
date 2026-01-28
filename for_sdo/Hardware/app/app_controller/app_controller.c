/*
 * app_controller.c
 *
 *  Created on: Jan 21, 2026
 *      Author: Administrator
 */
#include "app_controller.h"

Controller_t s_controller = { 0 };

void app_controller_init(void) {
	s_controller.led.statse = 0;
}

void app_controller_task(void) {
	static app_controller_task_statse_t app_controller_task_statse = APP_CONTROLLER_TASK_INIT;
	static uint32_t start_time = 0;

	switch (app_controller_task_statse) {
		case APP_CONTROLLER_TASK_INIT:
			start_time = HAL_GetTick();
			app_controller_init();
			app_controller_task_statse = APP_CONTROLLER_TASK_RUNNING;
			break;
		case APP_CONTROLLER_TASK_RUNNING:
			if (HAL_GetTick() - start_time > 1000)
			{
				start_time = HAL_GetTick();
				s_controller.led.statse = 1 - s_controller.led.statse;
			}
			break;
		default:
			app_controller_task_statse = APP_CONTROLLER_TASK_INIT;
			break;
	}
}
