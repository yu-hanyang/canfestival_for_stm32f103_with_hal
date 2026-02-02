/*
 * app_led.h
 *
 *  Created on: Jan 21, 2026
 *      Author: Administrator
 */

#ifndef APP_APP_LED_APP_LED_H_
#define APP_APP_LED_APP_LED_H_

#include "app_controller.h"
#include "main.h"

#define TURN_ON_LED 	(HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_SET))
#define TURN_OFF_LED 	(HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_RESET))

typedef enum {
APP_LED_TASK_INIT = 0,
APP_LED_TASK_RUNNING
}app_led_task_statse_t;

void app_led_task(void);

#endif /* APP_APP_LED_APP_LED_H_ */
