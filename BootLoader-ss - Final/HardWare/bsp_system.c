#include "bsp_system.h"



////Debug串口重定向
//#if 0
//#pragma import(__use_no_semihosting)
////标准库需要的支持函数
//struct __FILE
//{
//    int handle;
//};

//FILE __stdout;
////定义_sys_exit()以避免使用半主机模式
//void _sys_exit(int x)
//{
//    x = x;
//}
////重定义fputc函数
//int fputc(int ch, FILE *f)
//{
//    while((UART4->SR & 0X40) == 0); //循环发送,直到发送完毕

//    UART4->DR = (unsigned int) ch;
//    return ch;
//}

//#endif





//NOP 微秒级延时
void Delay_us(uint32_t UsCnt)
{
    uint32_t NopCnt = 0;
    uint32_t i = 0;

    for(i = 0; i < UsCnt; i++)
    {
        for(NopCnt = 0; NopCnt < 20; NopCnt++) //20个nop 1us
        {
            __NOP();
        }
    }
}





