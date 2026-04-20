#include "follow.h"
#include "interface.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "sensor.h"
#include "bsp_oled19264.h"
#include "main_logic.h"
#include "set.h"
#include "canreceive.h"
#include "bsp_ncv7356.h"
#include "bsp_485.h"
#include "main_logic.h"
#include "bsp_hd7279.h"
/***********************每行显示的数据***********************************/



uint8_t variant_Follow_Cutterposition;//采煤机位置
char array_Follow_Cutterposition[4];
uint8_t variant_Follow_Cutterderction;//采煤机运行方向
char array_Follow_Cutterderction[5];
u8 array_FOLLOW_moveCMD[8];
uint8_t state_Follow_movememory = 0;//跟机动作是否开始

void Follow_move(void)
{
	u8 array_R[8];
	u32 contorl_Valve;
	u8 i;
	u16 distance;
	u16 distance_M;
	u16 time;
	xQueueReceive(Follow_Automation_Queue,array_FOLLOW_moveCMD,portMAX_DELAY);
	if(_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)//默认状态下改变设备状态为被控
	{
		_variant_Mainlogic_statement.PRESENTstate_value = CONTROLED;
		State_main_TrumpetStatus = 2;
		i= 0;
		_variant_Interface_disp.type = 	TYPE_FOLLOW_MOVE;
	}
		
		switch(array_FOLLOW_moveCMD[2])			
		{
		
			case  FOLLOW_MOVE_SPRAY:
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = FOLLOW_MOVE_SPRAY;
			       Faction_Follow_UP(FOLLOW_MOVE_SPRAY,0,0,0x01);
					time =  array_FOLLOW_moveCMD[3]*10;
					contorl_Valve = MOVE_FUEL_SPRAY_OUT;
					while((i<time)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
						MOVE_controled_Driver(contorl_Valve);//降柱
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"      喷雾      ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
							}
							i++;

							vTaskDelay(100);

					}
					state_Follow_movememory = 0;
				}

			break;
			
			case FOLLOW_MOVE_PUSH://推溜
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{

			       Faction_Follow_UP(FOLLOW_MOVE_PUSH,0,0,0x01);
          
          //推溜行程需要重新改，改为每次推XXXXmm
					distance = array_FOLLOW_moveCMD[3];
					distance =	distance<<8;
					distance =	distance + 	array_FOLLOW_moveCMD[4];
					distance_M = distance+_sensor_distance.sensor_value;
					//distance = _sensor_distance.sensor_value - distance	;		
                    state_Follow_movememory = FOLLOW_MOVE_PUSH;
					contorl_Valve = MOVE_CHUTE_PUSH_OUT;//推溜
					while((i<70)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//降柱
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"      推溜      ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
							}
							if(_sensor_distance.sensor_value >= distance_M )
							{
                                     i=100;
							}
							i++;

							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}
						state_Follow_movememory = 0;
				       Faction_Follow_UP(FOLLOW_MOVE_PUSH,0,0,0x00);
					}

				break;
			
			case FOLLOW_MOVE_ADVANCE://移架
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
				    Faction_Follow_UP(FOLLOW_MOVE_ADVANCE,0,0,0x01);//向上位机汇报自动移架开始
					distance = array_FOLLOW_moveCMD[3];
					distance =	distance<<8;
					distance =	distance + 	array_FOLLOW_moveCMD[4];
					distance_M = distance;
         			 state_Follow_movememory = FOLLOW_MOVE_ADVANCE;
					contorl_Valve = 0;
					contorl_Valve = contorl_Valve|MOVE_COLUMN_DOWN_OUT;//降柱
					i = 0;
					while((i<20)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//降柱
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"      降柱      ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}
					i = 0;
					contorl_Valve = 0;
					contorl_Valve = MOVE_HYDRAULICFRAME_ADVANCING_OUT;

					while((i<70)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//移架
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"      移架      ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							if(_sensor_distance.sensor_value <= distance_M )
							{
                                     i=100;
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}
					i = 0;
					contorl_Valve = 0;
					contorl_Valve = MOVE_COLUMN_UP_OUT;
					while((i<20)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//sheng柱
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"      升柱      ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();
						
					}
					

			
						state_Follow_movememory = 0;
				        Faction_Follow_UP(FOLLOW_MOVE_ADVANCE,0,0,0x00);//向上位机汇报自动移架结束
					}

				
			
			
			
			
			
				break;
				
		
		
		
		
		
		
		
		
		
		
		}
	_variant_Interface_disp.type = 	TYPE_DEFAULT;
	_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
	State_main_TrumpetStatus = 0;
		LED_Management_Fanction(LED_NORMOL);
		xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);




}





void Faction_Follow_UP(uint8_t upper_num1,uint8_t upper_num2,uint8_t upper_num3,uint8_t State)
{
	
	uint8_t upper_message[7] = {0};
	  upper_message[0] = 0x21;
	  upper_message[1] = Structset_Service_DeviceNum1_1.Setting_Para;
	  upper_message[2] = upper_num1;
	  upper_message[3] = State;//0x00
	  upper_message[4] = 0x02;//跟机控制动作
	  upper_message[5] = upper_num2;
	  upper_message[6] = upper_num3;
	  CANx_SendNormalData(&BUS_CAN, 0xee, upper_message, 8);


}
























