# 串口打印工程
## 介绍
这个工程使用的是最小系统tcl脚本构建的，由于使用的是MIO，所以也没有引脚约束，Vitis使用的是`Hello World`例程，所以也没有Vitis代码（~~十分甚至有九分简洁~~
## 注意事项
- 从硬件介绍文件可以知道，这个串口实际上接的是PS的UART1（什么？你问UART0去哪了，UART0其实被BT1120排线接口引出来了），如果同时开了UART0和UART1的话，在Vitis开发中默认`printf/xilprintf`函数重定向的是UART0，为了使这个串口可以正常使用，我们需要在Vitis中做如下更改，`stdin`和`stdout`最好都改一下（如果是FreeRTOS则在Overview下面显示的是freertos）：
![串口重定向](https://github.com/Cube-Bay/Baidu_EdgeBoardLite_FZ3B_Vivado_Reference/blob/main/%E5%8F%82%E8%80%83%E5%B7%A5%E7%A8%8B/UART/UART%E5%9B%BE%E7%89%87%E7%B4%A0%E6%9D%90/Vitis%E4%B8%B2%E5%8F%A3%E9%87%8D%E5%AE%9A%E5%90%91.png)
- 但是刚上电的FSBL输出内容仍然重定向为UART0，如果你想看FSBL打印的内容的话（~~不跑Linux应该不会有人想看吧~~）需要在`system_wrapper`这个界面的`zynqmp_fsbl`的同样位置修改重定向的串口，在裸机和FreeRTOS下，FSBL的输出内容就是启动时最先打出来的几句话（时间、日期、版本号根据你实际操作的情况为主）：
    >Xilinx Zynq MP First Stage Boot Loader 
    >
    >Release 2021.2   Jan 11 2026  -  19:09:10
    >
    >PMU-FW is not running, certain applications may not be supported.
