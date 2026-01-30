canopen的sdo是客户端服务器模式，主站是客户端，从站是服务端，一发一收。
sdo分为快速sdo和普通sdo，两者的区别在于数据传输的多少。
快速sdo一次来回完成数据的传输，因此只能传输最大32位的数据
超过的就要使用普通sdo了
# 快速sdo
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/d36cea1ed7574df38c82b76b3350f902.png)
通过快速 SDO，可以直接对 CANopen 节点的对象字典中的值进行读取和修改，所以在
做参数配置之外，也经常作为关键性数据传输之用。比如 CANopen 控制机器人的电机转动
角度时，就使用 SDO 来传输，保证可靠到达。
## 规范工程
为了更好的进行实验，我们将工程代码整理规范一下，把mian函数中对canopen的初始化模块化
创建app_can.c 和app_can.h,将canopen的初始化放进去
```c
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



```

```c
/*
 * app_can.h
 *
 *  Created on: Jan 28, 2026
 *      Author: Administrator
 */

#ifndef APP_APP_CAN_APP_CAN_H_
#define APP_APP_CAN_APP_CAN_H_

#include "states.h"
#include "slavedic.h"
#include "cm3.h"
void app_can_init();

#endif /* APP_APP_CAN_APP_CAN_H_ */

```
对应的cm3模块也需要同步更改
```c
//cm3.c 新添加的
static CO_Data *co_data = NULL;

void canopen_node_begin()
{
	setNodeId(co_data, 1); // 设置Canopen id为1
	setState(co_data, Initialisation); // NMT状态设置为Initialisation
	setState(co_data, Pre_operational); // NMT状态设置为Pre_operational
	setState(co_data, Operational); // NMT状态设置为Operational
}

//cm3.h新添加的
void canopen_node_begin();

```
然后main.c把对应的初始化修改了
```c
  /* USER CODE BEGIN 2 */
	TIM2_Start();
	OLED_Init();

	OLED_ShowString(1, 1, "Rx :");
	OLED_ShowString(2, 1, "RxID:");
	OLED_ShowString(3, 1, "Leng:");
	OLED_ShowString(4, 1, "Data:");
	app_can_init();


  /* USER CODE END 2 */
```
完整代码[https://github.com/yu-hanyang/canfestival_for_stm32f103_with_hal/tree/main/for_sdo](https://github.com/yu-hanyang/canfestival_for_stm32f103_with_hal/tree/main/for_sdo)
## 实验
打开canopen字典生成工具，创建一个新的名字为slavedic的从站点，依然是用心跳，时间设置为三秒，配置如下图
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/eabda3338b0144cca348147b10be713e.png)
再设置一个用户变量为for_sdo![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/560b67eb997a44ea9836e52b075961fd.png)
把这个对象和tpdo1相关联
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f14990a4f3c44f99a49a13a8571a5566.png)
再设置TPDO的参数
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/9aa95fa5929a4a2da469a2ce5ce88b67.png)
生成字典，把生成的文件放入项目中，打开pc端的上位机可以观察到心跳报文和TPDO1的报文
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f15f1067d58841828f19e8f4b85a01ff.png)
### 可读可写 用户自定义索引
#### sdo单字节写实验
我们使用sdo命令修改我们刚刚定义的索引为2000的for_sdo。发送id为0x601 数据为2F 00 20 00 11 00 00 00的命令帧。可以观察到如下现象
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/0468cf4f1c11444d89eac508d9ec1642.png)
tpdo和预期一样被改为了11，有收到了来自从站的id为0x581回应。分解一下我刚刚发的命令
```c
-----------------------------------------------------------------------------
|        2F            |      00 20             |  00 |  11 00 00 00|
------------------------------------------------------------------------------
|cs命令，表示写入一个字节|索引（小端序，低字节在前）|子索引|数据（小端序）|
```
如果我们尝试读取该字节会怎么样？
#### sdo单字节读实验
使用id为601 数据为40 00 20 00 00 00 00 00 的数据帧
可以观察到
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/615aa3480426407598cbd6039568496e.png)
和预期一致，索引为2000的数据被读取了出来

该实验我们的用户索引是可读可写的，那如果该索引只读，而我们用sdo写，会发送什么现象。

### 只读 用户自定义索引
打开字典，修改0x2000的属性为只读
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/f54c2742b12d4c519aa29e7556905e90.png)
生成字典放入到项目中
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/343e02721b3d4d15805a0d9c42c5e51a.png)
可以观察到从站正在向外发送，和刚才可读可写没区别。
#### sdo单字节写实验
我们使用sdo命令修改我们刚刚定义的索引为2000的for_sdo。发送id为0x601 数据为2F 00 20 00 11 00 00 00的命令帧。可以观察到如下现象
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/e4a7467eda9c48a190a28248a99d12ee.png)
从站回复80，报错了
#### sdo单字节读实验
使用id为601 数据为40 00 20 00 00 00 00 00 的数据帧
可以观察到
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/0ee8f18673b047a4930b7d50a854f64e.png)
回复4F,成功读取
那如果该索引是只写的呢？会是什么现象

### 只读 用户自定义索引
打开字典，修改0x2000的属性为只写，这次我们给个初始值为12
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/17f4f9e1dc894592a7307c10070103a6.png)

![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/7c58cfb17f594503bb5b9485873a8b43.png)
可以观察到从站正在向外发送，和刚才可读可写没区别。
#### sdo单字节写实验
我们使用sdo命令修改我们刚刚定义的索引为2000的for_sdo。发送id为0x601 数据为2F 00 20 00 11 00 00 00的命令帧。可以观察到如下现象
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/d124f36eca0e4574a02048cf17b772cc.png)
可以观察到被改成功了
#### sdo单字节读实验
使用id为601 数据为40 00 20 00 00 00 00 00 的数据帧
可以观察到
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/3f096c0a943b48a3b9887b5be256f70e.png)
报错了，符合预期。
#### 结论
在快速sdo通信中，索引的可读可写会影响sdo对该索引的读写，但不会影响TPDO对该索引的发送。

### 可读可写 用户自定义索引 回调函数实验
使用字典生成工具配置0x2000 为可读可写并将回调函数生成勾选上
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/45d15bfb978e4c18994b61f97def1df9.png)
将生成的文件放入项目中
然后在canopen的初始化中添加回调函数
```c
//cm3.h
UNS32 Index2000_00callback(CO_Data *d, UNS16 wIndex, UNS8 bSubindex)
{
	for_sdo += 1;
	return 0;
}

void canopen_node_begin()
{
	RegisterSetODentryCallBack(co_data, 0x2000, 0x00, Index2000_00callback);
	setNodeId(co_data, 1); // 设置Canopen id为1
	setState(co_data, Initialisation); // NMT状态设置为Initialisation
	setState(co_data, Pre_operational); // NMT状态设置为Pre_operational
	setState(co_data, Operational); // NMT状态设置为Operational
}
```
我们使用sdo命令修改我们刚刚定义的索引为2000的for_sdo。发送id为0x601 数据为2F 00 20 00 17 00 00 00的命令帧。可以观察到如下现象
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/ecc64a2f44244234b3561f6aea99944f.png)
数值比我们给的+1，符合实验预期  
我们现在回调函数的返回值为0，假设我们给的返回值不是0，会发送什么现象？  
我们修改回调函数的代码
```c
//cm3.h
UNS32 Index2000_00callback(CO_Data *d, UNS16 wIndex, UNS8 bSubindex)
{
	for_sdo += 2;
	return 0xAA;
}
```
发送id为0x601 数据为2F 00 20 00 17 00 00 00的命令帧。可以观察到如下现象  
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/ea09cb9e74e2447e9b56f0afba139fc0.png)
虽然返回的命令是80开头的，提示异常响应，但是值还是被改了，而且返回的数据帧有携带我们回调函数的返回。
#### 结论
sdo的回调函数可以通过修改返回值来携带信息，最大为32位。