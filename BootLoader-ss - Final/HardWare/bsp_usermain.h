#ifndef __BSP_USERMAIN_H__
#define __BSP_USERMAIN_H__

#include "main.h"
#include "bsp_system.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"

//#include "stm32f4xx_flash.h"
//#include "sys.h"

extern void BaseInit(void);
extern void MainInit(void);
extern void MainWork(void);

extern uint32_t Program_Addr;
	


#endif
