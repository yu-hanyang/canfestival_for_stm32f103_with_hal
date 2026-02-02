/*
 * app_can.c
 *
 *  Created on: Jan 28, 2026
 *      Author: Administrator
 */
#include "app_can.h"

void app_can_init()
{
	canInit(&slavedic_Data, 500000);
	canopen_node_begin();
}
