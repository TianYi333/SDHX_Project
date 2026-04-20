#ifndef __BSP_NCV7356_H__
#define __BSP_NCV7356_H__

#include "stm32f4xx_hal.h"

#define LEFT_CAN 		hcan3
#define RIGHT_CAN 	hcan1
#define BUS_CAN			hcan2

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern CAN_HandleTypeDef hcan3;

extern uint16_t pro_time;


void CAN_Config_Ready(void);
void CAN_Config_Trans(void);

uint8_t CANx_SendNormalData(CAN_HandleTypeDef *hcanX, uint16_t ID, uint8_t *pData, uint16_t Len);

#endif
