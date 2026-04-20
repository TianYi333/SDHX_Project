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
uint8_t variant_Follow_Cutterderction;//采煤机位置
char array_Follow_Cutterderction[5];
u8 array_FOLLOW_moveCMD[8];
u16 distance;
uint8_t state_Follow_movememory = 0;//跟机动作是否开始    添加人任晓宇 添加日期2023/12/24
void Follow_move(void)
{
	u32 contorl_Valve;
	u8 i;
	u16 distance;
	u16 distance_M;
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
		
			case INLET_SAFEGUARD_BEGIN://伸护帮
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = INLET_SAFEGUARD_BEGIN;
			    Faction_Follow_UP(INLET_SAFEGUARD_BEGIN,0,0,0x01);
					contorl_Valve = 0;
					contorl_Valve = MOVE_PLATE_STRETCH_OUT;//伸护帮
					while((i<30)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"     伸护帮     ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();						
					}
					state_Follow_movememory = 0;
				  Faction_Follow_UP(INLET_SAFEGUARD_BEGIN,0,0,0x00);//伸护帮结束
				}
				break;
			
			case OUTLET_SAFEGUARD_BEGIN://收护帮
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = OUTLET_SAFEGUARD_BEGIN;
			    Faction_Follow_UP(OUTLET_SAFEGUARD_BEGIN,0,0,0x01);
					contorl_Valve = 0;
					contorl_Valve = MOVE_PLATE_WITHDRAW_OUT;//收护帮
					while((i<50)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//
						if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"     收护帮     ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}
					state_Follow_movememory = 0;
				  Faction_Follow_UP(OUTLET_SAFEGUARD_BEGIN,0,0,0x00);//伸护帮结束
				}
				break;
			
			case INLET_FRONTBEAM_BEGIN://伸前梁
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = INLET_FRONTBEAM_BEGIN;
			    Faction_Follow_UP(INLET_FRONTBEAM_BEGIN,0,0,0x01);
					contorl_Valve = 0;
					contorl_Valve = contorl_Valve|MOVE_TELESCOPIC_GIRDER_STRETCH_OUT;//伸前梁
					while((i<30)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//降柱
						if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"     伸前梁     ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}			
					state_Follow_movememory = 0;
				  Faction_Follow_UP(INLET_FRONTBEAM_BEGIN,0,0,0x00);//伸护帮结束		
				}
				break;
			
			case OUTLET_FRONTBEAM_BEGIN://收前梁
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = OUTLET_FRONTBEAM_BEGIN;
			    Faction_Follow_UP(OUTLET_FRONTBEAM_BEGIN,0,0,0x01);
					contorl_Valve = 0;
					contorl_Valve = contorl_Valve|MOVE_TELESCOPIC_GIRDER_WITHDRAW_OUT;//收护帮
					while((i<30)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//降柱
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"     收前梁     ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}		
					state_Follow_movememory = 0;
				  Faction_Follow_UP(OUTLET_FRONTBEAM_BEGIN,0,0,0x00);//伸护帮结束				
				}
				break;
			
			case PUSH_SCRAPER_BEGIN://推溜
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = PUSH_SCRAPER_BEGIN;
			    Faction_Follow_UP(PUSH_SCRAPER_BEGIN,0,0,0x01);
					 
          //推溜行程需要重新改，改为每次推XXXXmm
					distance = array_FOLLOW_moveCMD[3];
					distance =	distance<<8;
					distance =	distance + 	array_FOLLOW_moveCMD[4];
					distance_M = distance+_sensor_distance.sensor_value;
					//distance = _sensor_distance.sensor_value - distance	;	
					contorl_Valve = MOVE_CHUTE_PUSH_OUT;//推溜
					while((i<50)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
					{
							MOVE_controled_Driver(contorl_Valve);//降柱
							if((i%10)==0)
							{
								strcpy(variant_interface_SingalMove,"      推溜      ");
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							i++;
//							if(_sensor_distance.sensor_value>=distance)
//							{
//								i=31;
//							}
							vTaskDelay(100);
	//											Group_Pause_Count();

							
					}			
					state_Follow_movememory = 0;
					Faction_Follow_UP(PUSH_SCRAPER_BEGIN,0,0,0x00);			
				}
				break;
			
			case AUTO_MOVE_MACHINE_BEGIN://移架
				if(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED)
				{
					state_Follow_movememory = AUTO_MOVE_MACHINE_BEGIN;
			    Faction_Follow_UP(AUTO_MOVE_MACHINE_BEGIN,0,0,0x01);
					distance = array_FOLLOW_moveCMD[3];
					distance =	distance<<8;
					distance =	distance + 	array_FOLLOW_moveCMD[4];
					distance_M = distance;
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
					while((i<50)&&(_variant_Mainlogic_statement.PRESENTstate_value == CONTROLED))//降柱时间
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
					Faction_Follow_UP(AUTO_MOVE_MACHINE_BEGIN,0,0,0x00);//向上位机汇报自动移架结束	
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






















