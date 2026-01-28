

# 写在开头

最近学习了canopen，依据费曼学习法原理，特把移植过程与心得记录下来。内容若有问题，还望在评论区指出。实验环境：cubeide 1.19.0 + stm32f103c8t6(江协科技同款芯片，初学者友好)+江协科技can教程硬件（https://jiangxiekeji.com/tutorial/can.html ）。程序源码[https://github.com/yu-hanyang/canfestival_for_stm32f103_with_hal](https://github.com/yu-hanyang/canfestival_for_stm32f103_with_hal)
##  一些可能提前需要知道的点
### cubemx配置can需要整数？
在使用cubemx配置can的波特率和采样点时，查到Time quantum的配置必须为整数。参考自[STM32-STM32CubeMX的CAN波特率避坑](https://blog.csdn.net/qq_31094099/article/details/121533451)。而f103的can总线是在APB1上的，因此我把APB1的时钟配置为20MHz。
$CanTimeQuantum(can协议的一份的时间计算) = {(1 * prescaler )\over CanClocks}$ 我的分频系数为2 时钟为20mHz，因此计算结果为100ns。
$CanBraudrate(can波特率)={CanClocks \over (1+ BRP) * (1 + tq1 + tq2)}$BRP为实际配置分频系数减一，我tq1配置为16，tq2配置为3，因此计算结果为500k
$采样点={（1+tq1) \over (1+tq1+tq2)}$我将tq1配置为16将tq2配置为3，因此计算结果为85%
### canfestival对于硬件定时器的时间要求和硬件定时器的极限？
我在使用f103 TIM2定时器使发现该定时器怎么配置都达不到10us的速度，使用翻转引脚的电平再使用示波器观察波形发现，不管怎么样配置都到不了10us的速度，而在canfestival中是使用定时器控制数据的定时发送。参考自[https://www.cnblogs.com/wrddc/p/16413226.html](https://www.cnblogs.com/wrddc/p/16413226.html)该链接对于canfestival的Timer机制有详细的解释。为了使定时器和canfestvial的Timer匹配，可以通过修改Timecfg.h中的库对us和ms的两个宏定义，使其时间相匹配。

### sdo和pdo干货
#### can和canopen的关系
对于can传输信息有几个重要的概念，id和data。id分为标准id和拓展id，在canopen中我们重点关注11位的标准id。
canopen通过id来区分帧的功能，对于sdo来说，发送方以0x600+node_id（二进制为：110XXXXXXX,在canopen中最多只能有127个node）,作为id进行对外发送。而接收方则以0x580+node_id,作为id进行回应的。对于pdo来说，也是通过类似sdo的方法进行区分数据帧的。
#### 索引
canopen中有个叫索引的概念，这个概念在can中是没有的。这个索引可以理解为变量，这个变量有16位2字节的主索引（可以理解为面向对象中的对象或者结构体的地址）和8位一个字节的子索引（可以理解为面向对象中的成员或者结构体中的成员）。这个索引和can的关键联系在于can中的8个字节的data。回顾刚才说的sdo，我们只谈了id，现在关注8个字节的数据部分。该部分的第一个字节为cs命令符，第二、三字节为主索引，第四字节为子索引，第五到第八字节为该索引的子索引所传递的数据。
```
sdo数据段
-------------------------------------------------------------------------------------------------------
|cs命令符（占一个字节）|主索引（占两个字节，小端，也是低字节在前面）|子索引（占一个字节）|数据（占四个字节）|
----------------------------------------------------------------------------------------------------
```
然后pdo和sdo不大一样，pdo的数据端省略了命令、主索引和子索引。因此pdo数据端传输的数据是由通信的两方约定的。
canopen已经占用了一些索引用于控制sdo和pdo的运行参数（比如：pdo定时传输的时间）当然这些参数可以通过sdo进行修改，也可以在编写代码的时候就定下来（也就是**字典**，这个字典可以由工具生成）。
具体对于canopen对索引已经占用的部分和canopen具体的细节请参考[canopen轻松入门](https://www.zlg.cn/data/upload/software/Can/CANopen_easy_begin.pdf)
# cubemx配置
cubemx配置关键在于can和定时器的配置，这是canopen实现的关键。我需要把can配置成普通模式，500k的波特率。定时器则需要每一毫秒产生一次中断供canopen协议栈使用。
## can部分
can部分配置如下图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/8e7763db8f2c454c9fc6af9f5b9e4953.jpeg#pic_center)
接收被配置为中断接收
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/d0e77149d4e84757872e5c34380e451f.jpeg#pic_center)

## 定时器部分
定时器配置如下图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/3f189910ebdc4f98bbc87bcc2a960572.jpeg#pic_center)
还需要把定时器的中断函数给点上，因此需要配置nvic，如下图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/353f033ffbcd476b879e520ca9e674f9.jpeg#pic_center)

## clock configuration部分
配置如下
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/4496631a7d88473eaf5af4e886483f3e.png)
# 源码移植
源码地址：[
https://github.com/ljessendk/CanFestival](https://github.com/ljessendk/CanFestival)
先把源码下载下来
然后你需要在项目中建一个文件夹专门放置canfestival的文件
## src部分
把源码的src中的一些文件移植到项目中canfestival的文件夹中
需要转移的文件如下图红色加粗部分
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/fd2daecac14b4a8ab58a232c129be929.jpeg#pic_center)
## inc部分
把include部分的这些文件移植到项目中canfestival的文件夹中
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/af81f9fcbcd0464da0d141ebac007d41.jpeg#pic_center)
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/c5014226d8e24b55be347a7d16f5cf45.jpeg#pic_center)
再把include文件中cm4（指cotex m-4核）的文件夹移植到项目的canfestival的cm3（因为f103是m3核的所以这样命名）中，具体具体见下图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/c3e405dbd8ad44bda89125dc4c607854.jpeg#pic_center)
注意只移植cm4三个是不够的，因此需要把avr中的这个也放进去。
## driver部分
把源码driver中cm3的两个文件放到项目的canfestival的drv中
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/66d83831b20b4d02977c845be473abf4.jpeg#pic_center)
注意：这个和include中的cm3是不一样的
## 字典移植
源码中有一个工具（在源码的objdictgen目录下）可以帮我们生成字典文件，在项目的canfestival中另外建一个文件夹（我建的叫slavedic)用于存放生成的文件。具体的使用我参考了[https://blog.csdn.net/whahu1989/article/details/144226202](https://blog.csdn.net/whahu1989/article/details/144226202) ，可以移步该链接内容的后面部分，使用他的方法可以不使用python2，直接python3就行。
我用的是源码里的工具，环境配置参考：[
https://blog.csdn.net/lei_g/article/details/82823517](https://blog.csdn.net/lei_g/article/details/82823517)
使用方法：cmd进入源码objdictgen目录，然后键入python objdictedit.py即可启动该工具。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/b99efd532d9647ea8aa690ec63bc8cd8.jpeg#pic_center)
启动后，应该是这个界面。点击文件->新建
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/44e9112696754da29c09ac9c88be5101.jpeg#pic_center)
如上图配置
然后会生成一个界面，再如下图配置
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/bbb53c45322e4a48b5309e22f426ab12.jpeg#pic_center)
这个配置的意思是每3000（0x0BBB）毫秒发送一个心跳报文，然后我们需要配置几个用户索引
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/4cf5e72823ad4d7ca0bf99501e383e2b.jpeg#pic_center)点击添加，然后输入名字
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/a21c682a849844538c2aadebd6dc4e40.jpeg#pic_center)

就可以添加一个用户索引，为了将用户自定义的索引和TPDO联系起来，需要进pdo的配置
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/ff0d05cb77ef4d1a959d52c4224202db.jpeg#pic_center)


这样就可以把tpdo发送的数据和用户变量联系起来了，为了控制TPDo是如何发送到，需要配置如下
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/cf003279aac94de6aece9bfd3ef96cb6.jpeg#pic_center)
这样TPDO1将会以一定时间（0xFE:制造商异步事件），上面配置的是500毫秒（0x1F4）的周期向外发送。然后点击文件，导出词典
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/50cdde95513046c696fee52603d15886.jpeg#pic_center)
一共导出两个文件，.c .h  。把这两个文件放入刚才在项目canfestival中的slavedic文件夹中就可以了，但是我建议再按一下ctrl+s，把od文件也导出。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/4a57ade0fce049298824d13968c4f42b.jpeg#pic_center)
这样可以方便后续的继续开发，用字典生成工具打开这个od文件就可以在现有字典上进行二次修改了。所以使用完这个工具一共有三个文件。我在这个工具的演示中只添加了一共变量，大家可试着多加几个
# 代码部分
代码主要是需要修改driver中cm3.c（因为是用经典库实现的）和include/cm3的timerscfg.h(和定时器的时钟对其)，这两个文件。当然还需要对程序其他部分同步完善了。由于canfestival中存在can.h文件，我担心和cubemx生成的can.h产生冲突，就把名字改掉了（虽然我不知道不改会有什么影响，有大佬知道的还请在评论区赐教）
## timerscfg.h部分
```c
// The timer is incrementing every 10 us.
//#define MS_TO_TIMEVAL(ms) ((ms) * 100)
//#define US_TO_TIMEVAL(us) ((us) / 10)
#define MS_TO_TIMEVAL(ms) ((ms) * 1)
#define US_TO_TIMEVAL(us) ((us) / 1000)

```
观察源码的注释就知道这个是为10us的中断服务的，但是我们前期配置的是1ms的因此需要修改

## cm3.c部分
```c
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

//static CO_Data *co_data = NULL;
CAN_TxHeaderTypeDef CAN1_TxHeader;
CAN_RxHeaderTypeDef CAN1_RxHeader;

uint8_t canopen_rx_buffer[8] = { 0 }; //接收数组
uint8_t canopen_tx_buffer[8] = { 0 }; //发送数组

// Initializes the timer, turn on the interrupt and put the interrupt time to zero

//Set the timer for the next alarm.
void setTimer(TIMEVAL value) {
	NextTime = (TimeCNT + value) % TIMER_MAX_COUNT;
}

//Return the elapsed time to tell the Stack how much time is spent since last call.
TIMEVAL getElapsedTime(void) {
	TIMEVAL ret = 0;
	ret = TimeCNT >= last_time_set ?
			TimeCNT - last_time_set : TimeCNT + TIMER_MAX_COUNT - last_time_set;
	return ret;
}

// This function handles Timer 3 interrupt request.
void canopen_time(void) {
	TimeCNT++;
	if (TimeCNT >= TIMER_MAX_COUNT) {
		TimeCNT = 0;
	}
	if (TimeCNT == NextTime) {
		last_time_set = TimeCNT;
		TimeDispatch();
	}
}

// The driver send a CAN message passed from the CANopen stack
unsigned char canSend(CAN_PORT notused, Message *m) {
	uint32_t TxMailbox;
	CAN1_TxHeader.StdId = m->cob_id;                //标准id
	CAN1_TxHeader.ExtId = 0x0000;                  //扩展id
	CAN1_TxHeader.IDE = CAN_ID_STD;  //帧类型
	CAN1_TxHeader.DLC = (uint32_t) (m->len);  //数据长度
	CAN1_TxHeader.RTR = CAN_RTR_DATA; //数据帧
	if (HAL_CAN_AddTxMessage(&hcan, &CAN1_TxHeader, m->data, &TxMailbox)
			!= HAL_OK) {
		return 1;      //发送
	}
	return 0;
}


void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	Message Rx_Message = { 0 };      //CANopen需要用到的结构体
	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &CAN1_RxHeader,
			canopen_rx_buffer) != HAL_OK) {
		return;
	}
	//提取FIFO0中接收到的数据
	/* check the receive message */

	if (CAN1_RxHeader.IDE == CAN_ID_EXT) {      //扩展帧就跳出
		return;
	}
	Rx_Message.cob_id = CAN1_RxHeader.StdId;
	if (CAN1_RxHeader.RTR == CAN_RTR_REMOTE) {/* 标识符类型 */
		Rx_Message.rtr = 1;
	}
	Rx_Message.len = (uint8_t) (CAN1_RxHeader.DLC);/* 数据包长度 */
	memcpy(Rx_Message.data, canopen_rx_buffer, Rx_Message.len);/* 数据 */

	/* canopen数据包分配处理函数 */
	canDispatch(&slavedic_Data, &Rx_Message);


}
```
我只实现了上面几个函数，删了一些暂时用不到的，和源码中不大一样，源码中实现了对can外设的开启和配置，我们这边cubemx已经做完了配置因此不需要，需要关注一个canopen_time的函数，该函数需要放在我们最开始配置的TIM2定时器的中断函数中
```c
/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */
	HAL_GPIO_TogglePin(clock_test_GPIO_Port, clock_test_Pin);
	canopen_time();
  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}
```
(还可以看到我为了测试时钟的代码)，关于cm3.c中的代码编写参考了[https://www.cnblogs.com/tdyizhen1314/p/4348725.html](https://www.cnblogs.com/tdyizhen1314/p/4348725.html)，该博文是用经典库进行canfestvial的移植，感兴趣的小伙伴可以看一下
## cubemx生成的can.c
需要配置过滤器为全通，然后开启can
```c
/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 2;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_3TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_16TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

	CAN_FilterTypeDef sFilterConfig;
	sFilterConfig.FilterBank = 0; /* 过滤器组0 */
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK; /* 屏蔽位模式 */
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT; /* 32位。*/

	sFilterConfig.FilterIdHigh = 0x0000; /* 要过滤的ID高位 */
	sFilterConfig.FilterIdLow = 0x0000; /* 要过滤的ID低位 */
	sFilterConfig.FilterMaskIdHigh = 0x0000; /* 过滤器高16位每位必须匹配 */
	sFilterConfig.FilterMaskIdLow = 0x0000; /* 过滤器低16位每位必须匹配 */
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; /* 过滤器被关联到FIFO 0 */
	sFilterConfig.FilterActivation = ENABLE; /* 使能过滤器 */
	sFilterConfig.SlaveStartFilterBank = 14;

	 if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
	  {
	    /* Start Error */
	    Error_Handler();
	  }


	if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK) {
		/* Filter configuration Error */
		Error_Handler();

	}



	HAL_CAN_Start(&hcan);
  /* USER CODE END CAN_Init 2 */

}
```

## TIM部分
我添加了一个TIM2开启的函数
```c
/* USER CODE BEGIN 1 */
void TIM2_Start(void)
{
	  HAL_TIM_Base_Start_IT(&htim2);
}
/* USER CODE END 1 */
```

## 主函数部分
```c
  /* USER CODE BEGIN 2 */
	TIM2_Start();
	OLED_Init();

	OLED_ShowString(1, 1, "Rx :");
	OLED_ShowString(2, 1, "RxID:");
	OLED_ShowString(3, 1, "Leng:");
	OLED_ShowString(4, 1, "Data:");
	setNodeId(&slavedic_Data, 1); // 设置Canopen id为1
	setState(&slavedic_Data, Initialisation); // NMT状态设置为Initialisation
	setState(&slavedic_Data, Pre_operational); // NMT状态设置为Pre_operational
	setState(&slavedic_Data, Operational); // NMT状态设置为Operational


  /* USER CODE END 2 */
```
这样整体的移植就算完成了
# 实验现象部分
我这边单片机部分使用和江协科技同款硬件，只使用了一个节点。使用pc观察can报文需要特别的硬件，可以去淘宝买个兼容pcan的小板，或者买个zlg的can核。使用对应的上位机配置就可以观察到单片机定时发送的心跳和TPDO（注意：这个TPDO并非文中所配置的，文章中 的配置只是对字典工具的演示，当然你把文章中演示用户索引子索引的类型改成uint32，也可以达到同样的效果）
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/3048f7795e874d23801743e8ff09111a.png)
当然根据本文最开始说的，可以使用sdo报文对索引行修改
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f8b52816418f43c39bbf3ce925617a99.png)
输入以上命令（因为我对应TPDO的索引是0x2003），就可以更改用户索引的值，就可以观察到如下现象
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/764f3b71432246b8bd02ad0173ca41bc.png)
