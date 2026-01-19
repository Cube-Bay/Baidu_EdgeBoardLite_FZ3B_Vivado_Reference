# FreeRTOS下的TCP Server工程
## 介绍
网口作为一种有效的传输数据的手段，在各个板卡平台都有对应的硬件部署，百度大脑也不例外，板卡上提供了一个给PS使用的网口；

在Zynq上，我们使用[Lwip](https://www.cnblogs.com/kernelx/p/18173960)来实现PS侧的网络开发，尽管目前Lwip[官网](https://savannah.nongnu.org/projects/lwip/)已经更新到了2.2.1版本，但在21.2版Vitis中，我们仍然使用Lwip的2.1.1版本；

Lwip有三种不同的[开发方式](https://www.cnblogs.com/The-explosion/p/13582525.html)，在Vitis中支持裸机的RAW API和操作系统的Socket API，其中Socket API使用最为简单，但是代价是在Vitis中要使用FreeRTOS操作系统，不熟悉这个的话可以先看下[这个](https://gitee.com/xrbin/FreeRTOS_learning/tree/master/FreeRTOS10.4.6/01-%E7%AC%94%E8%AE%B0)，只看前几个基础知识就可以；

例程使用了AI生成，然后我进行了整理和精简，确保能跑起来，不了解流程的话可以问问AI（~~dicksuck，启动！~~
## 注意事项
在Vitis中Lwip是作为一种第三方库被加载进来的，这就意味着如果你创建的工程是Hello World模板，想使用Lwip就需要手动在`platform.spr`文件中勾选`lwip211`，不过就我个人而言不是很推荐这个做法，因为Lwip中有很多设置选项，比如`pbuf_pool_size`、`tcp_wnd`等参数，通过勾选加入Lwip的话，这些参数与默认参数一致，而默认的参数在传输数据比较大的时候Lwip就会卡死（~~然后疯狂找代码问题死活找不出来~~

所以这里我推荐在创建工程的时候不使用Hello World模板，而是直接使用官方提供的Lwip参考例程，在TCP中，官方提供了**回环服务器**、**服务器**、**客户端**三个例程，由于本工程是服务器，所以就使用官方的服务器例程来作为模板，建好之后再删掉官方源码就可以
## 添加网卡驱动
网口与Zynq芯片的连接并不是引脚直连的，这中间要经过物理层协议的Phy芯片，不严谨的说可以叫做我们平时常说的“网卡”，驱动网卡要通过读写网卡寄存器来实现，不同的网卡有不同的操作流程，具体要看手册，好在Lwip给我们提供了一些常用网卡的驱动，如果板子上的网卡属于其中某一个我们就不用操心网卡的问题，这个驱动在Vitis中位于`xxxxx_wrapper`->`psu_cortexa53_0`->`freertos10_xilinx_psu_cortexa53_0`->`bsp`->`psu_cortexa53_0`->`libsrc`->`lwip211_v1_6`->`src`->`contrib`->`ports`->`xilinx`->`netif`->`xemacpsif_physpeed.c`中，注意一定是`xemacpsif_physpeed.c`，因为这个目录下面还有个叫做`xaxiemacif_physpeed.c`的，那个是给PL侧走AXI的网口准备的的驱动，我们这里用不上

遗憾的是百度大脑上面用的Phy芯片KSZ9031RNXIC在Lwip中并没有对应驱动，好在这个芯片用的比较普遍，网上有[添加驱动的教程](https://blog.csdn.net/xinhh/article/details/140554494)，添加驱动有两种做法，一种是修改Vitis目录下面的文件，一种是直接改Vitis程序文件夹下面的源文件，教程中就是改的源文件，改Vitis源文件的好处是以后所有含Lwip的工程都会用改过的文件，不用手动再在工程中修改，缺点是一旦哪里改错了这个错误也会被一并加入，具体用那种看个人喜好，在上述教程中的自协商日志打印部分作者没有修改，我这里在他的基础上修改了一份，使用时直接用我的替换原来的源文件就可以