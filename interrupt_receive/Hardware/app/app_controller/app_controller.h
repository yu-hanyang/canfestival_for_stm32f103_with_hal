/*
 * app_controller.h
 *
 *  Created on: Jan 21, 2026
 *      Author: Administrator
 */

#ifndef APP_APP_CONTROLLER_APP_CONTROLLER_H_
#define APP_APP_CONTROLLER_APP_CONTROLLER_H_

#include "main.h"

typedef struct {
	uint8_t statse;
} LED_t;

typedef struct {
	LED_t led;
} Controller_t;

typedef enum{
	APP_CONTROLLER_TASK_INIT = 0,
	APP_CONTROLLER_TASK_RUNNING
}app_controller_task_statse_t;

extern Controller_t s_controller;

void app_controller_task(void);

#endif /* APP_APP_CONTROLLER_APP_CONTROLLER_H_ */
