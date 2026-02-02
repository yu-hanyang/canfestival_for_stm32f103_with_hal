项目代码地址：[https://github.com/yu-hanyang/canfestival_for_stm32f103_with_hal](https://github.com/yu-hanyang/canfestival_for_stm32f103_with_hal)
使用软件：cubeide、zcanpro(can核pc端上位机)、git
使用硬件：江协科技同款can教程硬件（stm32f103c8t6+TJA1050+st-link)、致远电子usbcan。
如果我的文档和项目有帮助到你，还请不要吝啬您的star。要是有什么问题，欢迎在评论区或者GitHub讨论区讨论

# 资源传送门
[canopen像亲妈一样贴心的入门指引（使用stm32f103作为主控，全程使用hal库+cubeide，使用canfestival作为canopen库）](https://blog.csdn.net/weixin_52091221/article/details/157546231?spm=1001.2014.3001.5502)

# PDO和索引的联系
pdo是canopen入门的关键移一步，这是canopen的灵魂。  
我喜欢把pdo比喻为一辆火车，这辆火车一共有8节车厢，每一节车厢最多携带一个字节的“货物”。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/11488ce21807474c846eb8c54e9491f3.png)
再把索引比喻为某个公司的货物，一个公司占用数节车厢。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/7e02ce94f3a2447eb03e1c233acf86c0.png)
注意：图片上的字节数是不对的，应为8个字节。
不同的公司有不同数据，对应用户索引的数据
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/ff8278007e674ce98bb7884cafbcadce.png)
不同货物是怎么放在这辆pdo火车上，是由该pdo的装货公司决定的，对应pdo mapping
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/fc90e9956fa34dcaafb29b54b063cfef.png)
这辆火车是怎么开的，如异步开，还是同步开，多久开一次，也是由该pdo的驾驶公司决定的，对应pdo parameter
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/5a7ffc8383bb421086d3198dd2a9c24c.png)
到这儿，我们有了三个概念，索引，pdo mapping ，pdo parameter。我们把索引比作公司，该公司有不同的货物需要pdo运输。把pdo mapping比喻装货公司，该公司决定了货物在pdo上的排布方式。把pdo parameter比作驾驶公司，该公司决定了pdo是怎么发送的。因此我们发现一个pdo需要两个公司来决定，然后火车对应入站的和出站的，对应为rpdo和tpdo。具体来说TPDO1需要有装货公司（对应字典索引为0x1A00）和一个驾驶公司（对应字典索引为0x1800）。   那这些canopen的概念是如何与can联系在一起的呢？这些pdo的can_id是固定的，每一辆pdo火车都有自己的轨道。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/c6d149749cfe4b7fae56b9ed23f46519.png)

相信到这里，你的pdo有一定概念了。现在我们有4个字节的阿里巴巴的货物，2个字节的百度货物，需要放到pdo火车上往外发（TPDO1），该火车每3秒发动一次。
# 定时发送实验
复制上一节的sdo工程改名为pdo_test。然后我们直接用字典生成工具打开工程字典.od文件
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/0e82654404c5499da533769da4340040.png)
把上一次的用户变量删了，添加uint32类型的阿里巴巴变量初始值为0x11223344和uint16的百度变量初始值为0x1122。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/238275a0dc174904b864fc2eaa74097c.png)
然后，设定pdo装货公司的参数（0x1A00)
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/c9e0a4ae2fc041d792cb36d016cd5c8c.png)
最后设定驾驶公司的参数，Transmission Type 为0xFE(异步，制造商特定事件),Event Timer为0xBB8（事件定时器触发的时间(单位 ms)设置为3000ms）
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/8b84f24255a64c9a9739a90318914c82.png)
然后把词典导入该工程，编译烧录。编译应该是过不了的，因为这个工程是从for_sdo改造的，所以需要把工程中遗留的回调函数删除了，再编译烧录。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/720b2d4d9d8a471a8616b03c3b8f6214.png)
打开pc端的上位机应该可以看到如上现象，id为0x181的数据帧正在定时往外发送。
# RPDO接收实验
打开字典生成工具，将两个用户参数映射到rpdo1
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/63f586b9b3d642769b24d7831c383e3c.png)

在把Rpdo的配置参数和TPDO一样
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/8b8af65ad9c44fab907231b8365a200c.png)
然后就可以用上位机发送id为201 数据为11 11 11 11 11 11 00 00 的数据帧，去更改对应映射位置的索引的值。
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/a28ad652812f48dda0a86978a6c6a737.png)
上面的两种通过观察pdo的配置不难发现是异步方式传输的，为了避免可以有效避免异步传输导致的应用逻辑混乱和总线负载不平衡的问题，需要使用同步方式传输。
# 同步循环传输
打开字典，将tpdo的配置设置为01
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/1534cd29a5284c7f86f4a1426eeadf82.png)
顺便将rpdo的配置也同步改了
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/a8e1f608ef30433fa5e3807e59c3afea.png)
用pc上位机发送80同步帧
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/85f7da0f5c0f4ffc8664027b247307f1.png)
可以看到每发送一次80，从站就发送一次tpdo
用上位机发送id为201 数据为11 11 11 11 11 11的数据帧
![在这里插入图片描述](https://i-blog.csdnimg.cn/direct/51a4e47f00f04c9ba51d8b10588ad82d.png)
可以看到rpdo的传值也是有效果的。