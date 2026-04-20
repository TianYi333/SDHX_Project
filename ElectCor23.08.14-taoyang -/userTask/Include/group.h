#ifndef __BUS_MANAGE_H__
#define __BUS_MANAGE_H__
#include "stm32f4xx_hal.h"



uint8_t *Display_Group_Movedirectionjudge(uint8_t direction_flag,uint8_t Separated_Num,uint8_t Action_Num);
void Group_Move_Control(uint8_t num);
void Faction_group_UP(uint8_t upper_num1,uint8_t upper_num2,uint8_t upper_num3,uint8_t State);
#endif



