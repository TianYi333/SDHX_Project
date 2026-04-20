#ifndef __FOLLOW_H__
#define __FOLLOW_H__
#include "stm32f4xx_hal.h"

extern uint8_t variant_Follow_Cutterposition;//采煤机位置
extern char array_Follow_Cutterposition[4];
extern uint8_t variant_Follow_Cutterderction;//采煤机位置
extern char array_Follow_Cutterderction[5];
void Follow_move(void);
void Faction_Follow_UP(uint8_t upper_num1,uint8_t upper_num2,uint8_t upper_num3,uint8_t State);


#define    FOLLOW_MOVE_PUSH        0x31          //跟机推溜（开始）


#define    FOLLOW_MOVE_ADVANCE        0x32            //跟机自动移架（开始）
#define    FOLLOW_MOVE_SPRAY        0x33            //跟机喷雾（开始）

extern uint8_t state_Follow_movememory;//跟机动作是否开始
#endif



