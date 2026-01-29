// Includes for the Canfestival driver
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_it.h"
#include "tim.h"
#include "gpio.h"
#include "can.h"
#include "canfestival.h"
#include "timer.h"
#include "cm3.h"
#include "fcan.h"
#include "slavedic.h"

/* Gestion d'un buffer tournant de messages CAN en Tx pour que le transfert
 * en mode block fonctionne avec SDO_BLOCK_SIZE > 3 
 */
#define TIMER_MAX_COUNT                         0xFFFF

//static uint8_t tx_fifo_in_use = 0;

static TIMEVAL last_time_set = TIMER_MAX_COUNT;
static unsigned int NextTime = 0;
static unsigned int TimeCNT = 0;

static CO_Data *co_data = NULL;
CAN_TxHeaderTypeDef CAN1_TxHeader;
CAN_RxHeaderTypeDef CAN1_RxHeader;

uint8_t canopen_rx_buffer[8] =
{
		0 }; //接收数组
uint8_t canopen_tx_buffer[8] =
{
		0 }; //发送数组

// Initializes the timer, turn on the interrupt and put the interrupt time to zero

//Set the timer for the next alarm.
void setTimer(TIMEVAL value)
{
	NextTime = (TimeCNT + value) % TIMER_MAX_COUNT;
}

//Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void)
{
	TIMEVAL ret = 0;
	ret = TimeCNT >= last_time_set ?
			TimeCNT - last_time_set : TimeCNT + TIMER_MAX_COUNT - last_time_set;
	return ret;
}

// This function handles Timer 3 interrupt request.
void canopen_time(void)
{
	TimeCNT++;
	if (TimeCNT >= TIMER_MAX_COUNT)
	{
		TimeCNT = 0;
	}
	if (TimeCNT == NextTime)
	{
		last_time_set = TimeCNT;
		TimeDispatch();
	}
}

// The driver send a CAN message passed from the CANopen stack
unsigned char canSend(CAN_PORT notused, Message *m)
{
	uint32_t TxMailbox;
	CAN1_TxHeader.StdId = m->cob_id;                //标准id
	CAN1_TxHeader.ExtId = 0x0000;                  //扩展id
	CAN1_TxHeader.IDE = CAN_ID_STD;  //帧类型
	CAN1_TxHeader.DLC = (uint32_t) (m->len);  //数据长度
	CAN1_TxHeader.RTR = CAN_RTR_DATA; //数据帧
	if (HAL_CAN_AddTxMessage(&hcan, &CAN1_TxHeader, m->data, &TxMailbox)
			!= HAL_OK)
	{
		return 1;      //发送
	}
	return 0;
}
//Initialize the CAN hardware
unsigned char canInit(CO_Data *d, uint32_t bitrate)
{
	// /* save the canfestival handle */
	co_data = d;

	return 1;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	Message Rx_Message =
	{
			0 };      //CANopen需要用到的结构体
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CAN1_RxHeader,
			canopen_rx_buffer) != HAL_OK)
	{
		return;
	}
	//提取FIFO0中接收到的数据
	/* check the receive message */

	if (CAN1_RxHeader.IDE == CAN_ID_EXT)
	{      //扩展帧就跳出
		return;
	}
	Rx_Message.cob_id = CAN1_RxHeader.StdId;
	if (CAN1_RxHeader.RTR == CAN_RTR_REMOTE)
	{/* 标识符类型 */
		Rx_Message.rtr = 1;
	}
	Rx_Message.len = (uint8_t) (CAN1_RxHeader.DLC);/* 数据包长度 */
	memcpy(Rx_Message.data, canopen_rx_buffer, Rx_Message.len);/* 数据 */

	/* canopen数据包分配处理函数 */
	canDispatch(co_data, &Rx_Message);

}

UNS32 Index2000_00callback(CO_Data *d, UNS16 wIndex, UNS8 bSubindex)
{
	for_sdo += 2;
	return 0x33221100;
}

void canopen_node_begin()
{
	RegisterSetODentryCallBack(co_data, 0x2000, 0x00, Index2000_00callback);
	setNodeId(co_data, 1); // 设置Canopen id为1
	setState(co_data, Initialisation); // NMT状态设置为Initialisation
	setState(co_data, Pre_operational); // NMT状态设置为Pre_operational
	setState(co_data, Operational); // NMT状态设置为Operational
}


