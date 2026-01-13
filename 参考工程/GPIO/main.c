#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "xparameters.h"
#include "xgpio.h"

// 定义AXI-GPIO结构体
XGpio LED_GPIO;

int main()
{
    init_platform();

    // 初始化AXI-GPIO
    XGpio_Initialize(&LED_GPIO , XPAR_AXI_GPIO_0_DEVICE_ID);
    // 设置数据方向（输入/输出）
    XGpio_SetDataDirection(&LED_GPIO , 1 , 0);

    while(1){
    	// 点亮LED
    	XGpio_DiscreteWrite(&LED_GPIO , 1 , 0);
    	sleep(1);
    	// 熄灭LED
    	XGpio_DiscreteWrite(&LED_GPIO , 1 , 1);
    	sleep(1);
    }

    cleanup_platform();
    return 0;
}
