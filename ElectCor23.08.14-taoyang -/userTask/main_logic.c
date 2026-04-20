#include "main_logic.h"
#include "stm32f4xx_hal.h"
#include "interface.h"
#include "sensor.h"
#include "bsp_hd7279.h"
#include "main.h"
#include "set.h"
#include "parameter_operation.h"
#include "controlded.h"
#include "sensor.h"
#include "bsp_mcz33996.h"
#include "drv_usart.h"
#include "bsp_oled19264.h"
#include "canreceive.h"
#include "group.h"
#include "bsp_ncv7356.h"
/***************
   URGENCY_STOP    0x00
  NORMOL_IDLE     0x01
  AUTO_IDLE       0x02
  SETTING       0x03
  CONTROLED       0x04
 FREEZE       0x05
  MOVE       0x06
 URGENCY_LOCK       0x0a

	u8 singledirection;		//单动方向
	u8 singlemovetype ;		//单动动作类型
	u8 singlenum;		//  控制到第几架单动
	u8 singlecancle;		//控制途中取消对其他的支架控制
	u8 singlemoveCMD;		//单动动作命令
_variant_Mainlogic_CMD.singledirection

#define   SINGAL_CAMCEI    0x00
#define   SINGAL_PITCH_ON     0x01
#define   SINGAL_MOVE       0x03

enum Mainlogic_STATEMENT{DWFAULT=0,Left_SINGLE=0x01,Right_SINGLE,Left_GROUP,Right_GROUP,Left_SETTING,Right_SETTING};
******************/
      
enum Mainlogic_STATEMENT{MAINLOGIC_DWFAULT=0,MAINLOGIC_SINGLE=0x01,MAINLOGIC_GROUP=0x02,MAINLOGIC_SETTING=0x03,MAINLOGIC_WORKINGCONDITION = 0x04};//主控控制模式
enum  Mainlogic_STATEMENT State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;

uint8_t variant_Mainlogic_KeyValue;//采集键值变量  创建人蒋博 创建日期2023/01/8
uint8_t variant_Mainlogic_Qreturn;//延时按键检测变量  创建人蒋博 创建日期2023/01/15
uint8_t variant_Mainlogic_Setting_subdirectory;//设置界面横向子菜单  创建人蒋博 创建日期2023/02/9
uint8_t const_Mainlogic_InterfaceFlag =1;//屏显更新指令  创建人蒋博 创建日期2023/01/17
/**************************************************************状态变量定义**************************************************************/
uint8_t State_Mainlogic_Beepcommand;//传递蜂鸣器变量  创建人蒋博 创建日期2023/01/8
uint8_t State_Mainlogic_DWFAULTstate = NORMOL_IDLE;//默认空闲状态变量  创建人蒋博 创建日期2023/01/8
/**********************************************数组定义*******************************************************/
uint8_t array_Mainlogic_KEYcommand[8];//传递命令变量  创建人蒋博 创建日期2023/01/8

uint8_t singnal_Mainlogic_KEY= 1;
uint8_t Count_Mainlogic_CanSingal= 0;//单动can发送计时变量
uint8_t Count_Mainlogic_workingCondition= 0;//工况刷新
uint8_t Flag_Mainlogic_croupmovecancel = 0;//接收数组
uint8_t Flag_Mainlogic_Groupcontorl = 0;//该设备有一个需负责的成组控制
uint8_t variant_Mainlogic_GroupMovenum = 0;//该设备有一个需负责的成组控制
uint8_t  Group_head;
uint8_t  Group_tail;
/*******************************************对外声明************************************************************************************************************/
struct _variant_statement _variant_Mainlogic_statement; 
struct _variant_mainlogicCMD1  Mainlogic_singalCMD;
struct _variant_mainlogicCMD2  Mainlogic_groupCMD; 
union _unit_mainlogic_Group _unitGroupCMD;
union _unit_mainlogic_Singal _unitSingalCMD;

void Main_Logic_SingalMove(uint8_t Key_Value);

void Main_logic(void)
{
	
	BaseType_t xReturn;
	uint8_t erro_Flag = 0;
	uint8_t Group_Num_Record = 0;//成组动作范围
	uint8_t *p;//接收数组
	uint8_t  array_cancel[5]={0};//用于成组暂停和取消

		LED_Management_Fanction(LED_BLINK);
		 switch(State_Mainlogic_pKEYstu)
		 {
			 
			 case MAINLOGIC_DWFAULT:
				xQueueReceive(Key_Value_ISR_Queue,&variant_Mainlogic_KeyValue,portMAX_DELAY);

				 switch(variant_Mainlogic_KeyValue)
				{
					
						/**
							array_Mainlogic_KEYcommand[0] = 0x01;//右邻架单动//0x02左邻架单动
							array_Mainlogic_KEYcommand[1] = 0;//动作支架1隔架2隔一架//以此类推
							array_Mainlogic_EYcommand[2] 		//0取消选中//1选中//2动作//3取消动作
							array_Mainlogic_KEYcommand[3] = 低位动作类型/高位0降收1升伸				
						**/
						case KEY3_VALUE:
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{
								State_main_TrumpetStatus = 1;
								variant_Mainlogic_Qreturn = 1;
					/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
					/********************************改变设备状态**********************************/
								State_Mainlogic_pKEYstu=MAINLOGIC_SINGLE;// 向右单动状态
							/********************************发布单动控制选中命令**********************************/
								_unitSingalCMD.Mainlogic_singalCMD.type =TPYE_SINGAL;
								_unitSingalCMD.Mainlogic_singalCMD.direction = DIRECTION_RIGHT;//右邻架单动	
								_unitSingalCMD.Mainlogic_singalCMD.movetype= SINGAL_PITCH_ON;//选中
								_unitSingalCMD.Mainlogic_singalCMD.num= 1;//距离
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD= MOVE_NONE;
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								_variant_Interface_disp.cursor  = 1;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								variant_Mainlogic_KeyValue = 0;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								LED_Management_Fanction(LED_SINGAL);
							}

							break;
						case KEY7_VALUE:
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{
								State_main_TrumpetStatus = 1;
								variant_Mainlogic_Qreturn = 1;
					/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
					/********************************改变设备状态**********************************/
								State_Mainlogic_pKEYstu=MAINLOGIC_SINGLE;// 向右单 动状态
						/********************************发布单动控制选中命令**********************************/
								_unitSingalCMD.Mainlogic_singalCMD.type =TPYE_SINGAL;
								_unitSingalCMD.Mainlogic_singalCMD.direction = DIRECTION_LEFT;//右邻架单动
								_unitSingalCMD.Mainlogic_singalCMD.movetype= SINGAL_PITCH_ON;//动作支架
								_unitSingalCMD.Mainlogic_singalCMD.num= 1;//距离
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD= MOVE_NONE;
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								_variant_Interface_disp.cursor  = 1;							
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								variant_Mainlogic_KeyValue = 0;	
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								LED_Management_Fanction(LED_SINGAL);
							}
							break;

						case KEY4_VALUE://右边电液控成组控制	
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{

								LED_Management_Fanction(LED_GROUPRIGHT);
								State_main_TrumpetStatus = 1;
								variant_Mainlogic_Qreturn = 1;
						/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
					/********************************改变设备状态**********************************/
								State_Mainlogic_pKEYstu=MAINLOGIC_GROUP;// 向右单 动状态
						/********************************发布单动控制选中命令**********************************/
								_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP_ALARM;
								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_R_RIGHT;
								_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_ADVANCING;	
								p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Framemove_GroupSeparatedNum24_1.Setting_Para,Structset_Framemove_GroupRange25_1.Setting_Para);
								_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
								_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
								_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
								Group_head =  *(p+2);
								Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
								_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Framemove_GroupSeparatedNum24_1.Setting_Para;
								_unitGroupCMD.Mainlogic_groupCMD.controlnum7 =  Structset_Service_DeviceNum1_1.Setting_Para;
								_variant_Interface_disp.type = 	TYPE_GROUP_RIGHT;
								
								variant_interface_move[0] = 0xD2;
								variant_interface_move[1] = 0xC6;
								variant_interface_move[2] = 0xBC;
								variant_interface_move[3] = 0xDC;
								variant_interface_move[4] = '\0';
								variant_interface_move[5] = '\0';
								variant_interface_move[6] = '\0';
								
								variant_interface_Grouptail[0] = (Group_tail/100)+0x30;
								variant_interface_Grouptail[1] = ((Group_tail%100)/10)+0x30;
								variant_interface_Grouptail[2] = (Group_tail%10)+0x30;
								variant_interface_Grouptail[3] = '\0';
								variant_interface_Grouphead[0] = (Group_head/100)+0x30;
								variant_interface_Grouphead[1] = ((Group_head%100)/10)+0x30;
								variant_interface_Grouphead[2] = (Group_head%10)+0x30;
								variant_interface_Grouphead[3] = '\0';
								
								variant_interface_arrows[0] = '-';
								variant_interface_arrows[1] = '-';
								variant_interface_arrows[2] = '>';
								variant_interface_arrows[3] = '\0';
								
								if(*(p+3)==1)//如果没有可成组架号
								{
									State_main_TrumpetStatus = 0 ;
									/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							        _variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
					/********************************改变设备状态**********************************/
								    State_Mainlogic_pKEYstu=MAINLOGIC_DWFAULT;// 向右单 动状态
						/********************************发布单动控制选中命令**********************************/
									_variant_Interface_disp.type = TYPE_DEFAULT;
									LED_Management_Fanction(LED_NORMOL);
									
								}
								
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
							}
							break;
						
							
							
						case KEY8_VALUE://右边电液控成组控制	
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{
								LED_Management_Fanction(LED_GROUPLEFT);
								State_main_TrumpetStatus = 1;
								variant_Mainlogic_Qreturn = 1;
						/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
					/********************************改变设备状态**********************************/
								State_Mainlogic_pKEYstu=MAINLOGIC_GROUP;// 向右单 动状态
						/********************************发布单动控制选中命令**********************************/
								_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP_ALARM;
								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
								_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_ADVANCING;	
								p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Framemove_GroupSeparatedNum24_1.Setting_Para,Structset_Framemove_GroupRange25_1.Setting_Para);
								_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
								_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
								_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
								Group_head =  *(p+2);
								Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
								_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Framemove_GroupSeparatedNum24_1.Setting_Para;
								_unitGroupCMD.Mainlogic_groupCMD.controlnum7 =  Structset_Service_DeviceNum1_1.Setting_Para;
								_variant_Interface_disp.type = 	TYPE_GROUP_LEFT;
								
								variant_interface_move[0] = 0xD2;
								variant_interface_move[1] = 0xC6;
								variant_interface_move[2] = 0xBC;
								variant_interface_move[3] = 0xDC;
								variant_interface_move[4] = '\0';
								variant_interface_move[5] = '\0';
								variant_interface_move[6] = '\0';
								
								variant_interface_Grouphead[0] = (Group_tail/100)+0x30;
								variant_interface_Grouphead[1] = ((Group_tail%100)/10)+0x30;
								variant_interface_Grouphead[2] = (Group_tail%10)+0x30;
								variant_interface_Grouphead[3] = '\0';
								variant_interface_Grouptail[0] = (Group_head/100)+0x30;
								variant_interface_Grouptail[1] = ((Group_head%100)/10)+0x30;
								variant_interface_Grouptail[2] = (Group_head%10)+0x30;
								variant_interface_Grouptail[3] = '\0';
								
								variant_interface_arrows[0] = '<';
								variant_interface_arrows[1] = '-';
								variant_interface_arrows[2] = '-';
								variant_interface_arrows[3] = '\0';
								
								if(*(p+3)==1)//如果没有可成组架号
								{
									State_main_TrumpetStatus = 0;
									/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							        _variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
					/********************************改变设备状态**********************************/
								    State_Mainlogic_pKEYstu=MAINLOGIC_DWFAULT;// 向右单 动状态
						/********************************发布单动控制选中命令**********************************/
									_variant_Interface_disp.type = TYPE_DEFAULT;
									LED_Management_Fanction(LED_NORMOL);
									
								}
								
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								

								
							}
							break;
					  //J
						/**********************自动推溜**********************/
						case KEY16_VALUE:	//按下启动按键，开始自动自动降-移-升控制
						if((Flag_Mainlogic_Groupcontorl != 0)&&(Flag_Mainlogic_croupmovecancel == 1))//开始
						{
									array_cancel[0]=TPYE_GROUP_CANAEL;
									array_cancel[1]= Flag_Mainlogic_Groupcontorl;
									array_cancel[2] = 0x03;
									array_cancel[3] = variant_Mainlogic_GroupMovenum;
									array_cancel[4] = Structset_Service_DeviceNum1_1.Setting_Para;
									xQueueSend(CAN_Send_Queue,array_cancel,0);
									Flag_Mainlogic_croupmovecancel=0;
						}

						break;
					case KEY12_VALUE:	//取消键
						if(Flag_Mainlogic_Groupcontorl != 0)//暂停
						{
							if(Flag_Mainlogic_croupmovecancel == 0)
							{
								array_cancel[0]= TPYE_GROUP_CANAEL;
								array_cancel[1]= Flag_Mainlogic_Groupcontorl;
								array_cancel[2] = 0x01;
								array_cancel[3] = variant_Mainlogic_GroupMovenum;
								array_cancel[4] = Structset_Service_DeviceNum1_1.Setting_Para;
								xQueueSend(CAN_Send_Queue,array_cancel,0);
							
								Flag_Mainlogic_croupmovecancel =1;//进入取消界面
							
							}
							else if(Flag_Mainlogic_croupmovecancel == 1)//取消
							{
								array_cancel[0]=TPYE_GROUP_CANAEL;
								array_cancel[1]= Flag_Mainlogic_Groupcontorl;
								array_cancel[2] = 0x02;
								array_cancel[3] = variant_Mainlogic_GroupMovenum;
								array_cancel[4] = Structset_Service_DeviceNum1_1.Setting_Para;
								xQueueSend(CAN_Send_Queue,array_cancel,0);
							
								
							}
						
						
						}
							break;
							/*********************设置界面*********************/
						case KEY15_VALUE://工况
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{
						/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
								   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
						/********************************改变设备状态**********************************/
									State_Mainlogic_pKEYstu = MAINLOGIC_WORKINGCONDITION;
									_variant_Interface_disp.type = 	TYPE_WORKINGCONDITION_WORKINGCONDITION1;
									_variant_Interface_disp.cursor  = 0;
									variant_Mainlogic_KeyValue = 0;	
									Count_Mainlogic_workingCondition = 0;
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							break;
						case KEY19_VALUE:
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{
								State_main_TrumpetStatus = 1;

								variant_Mainlogic_Qreturn = 1;
					/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
					/********************************改变设备状态**********************************/
								State_Mainlogic_pKEYstu = MAINLOGIC_SETTING;
								_variant_Interface_disp.cursor  = 1;
								_variant_Interface_disp.type = 	TYPE_SETTING_GLOBAL_SET;
								variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GLOBAL_SET_LIMT;
								variant_Mainlogic_KeyValue = 0;	
								variant_Mainlogic_Setting_subdirectory = 0;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								LED_Management_Fanction(LED_SETING);
							}
							break;
						
						case KEY23_VALUE:
							if((_variant_Mainlogic_statement.PRESENTstate_value == State_Mainlogic_DWFAULTstate)||(_variant_Mainlogic_statement.PRESENTstate_value == URGENCY_LOCK))
							{
								State_main_TrumpetStatus = 1;

								variant_Mainlogic_Qreturn = 1;
					/********************************改变设备受控状态---极为重要，本任务只有一对**********************************/
							   _variant_Mainlogic_statement.PRESENTstate_value = SETTING;
					/********************************改变设备状态**********************************/
								State_Mainlogic_pKEYstu = MAINLOGIC_SETTING;
								_variant_Interface_disp.cursor  = 1;
								_variant_Interface_disp.type = 	TYPE_SETTING_FAULT;
								variant_Mainlogic_KeyValue = 0;	
								variant_Mainlogic_Setting_subdirectory = 10;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								LED_Management_Fanction(LED_SETING);
							}
							break;
						
					
					default:
						break;
					

				}
            	xQueueReset(Key_Value_ISR_Queue);
					break;
			 case MAINLOGIC_SINGLE:
					if(_unitSingalCMD.Mainlogic_singalCMD.direction == DIRECTION_RIGHT)
					{
						switch(_unitSingalCMD.Mainlogic_singalCMD.num)
						{
							case 1:
								variant_interface_SingalLR[0] = 0xD3;
								variant_interface_SingalLR[1] = 0xD2;
								variant_interface_SingalLR[2] = 0xC1;
								variant_interface_SingalLR[3] = 0xDA;
								variant_interface_SingalLR[4] = 0xBC;
								variant_interface_SingalLR[5] = 0xDC;
								variant_interface_SingalLR[6] = 0x20;
								variant_interface_SingalLR[7] = 0x20;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 2:
								variant_interface_SingalLR[0] = 0xD3;
								variant_interface_SingalLR[1] = 0xD2;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xBC;
								variant_interface_SingalLR[5] = 0xDC;
								variant_interface_SingalLR[6] = 0x20;
								variant_interface_SingalLR[7] = 0x20;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 3:
								variant_interface_SingalLR[0] = 0xD3;
								variant_interface_SingalLR[1] = 0xD2;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xD2;
								variant_interface_SingalLR[5] = 0xBB;
								variant_interface_SingalLR[6] = 0xBC;
								variant_interface_SingalLR[7] = 0xDC;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 4:
								variant_interface_SingalLR[0] = 0xD3;
								variant_interface_SingalLR[1] = 0xD2;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xB6;
								variant_interface_SingalLR[5] = 0xFE;
								variant_interface_SingalLR[6] = 0xBC;
								variant_interface_SingalLR[7] = 0xDC;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 5:
								variant_interface_SingalLR[0] = 0xD3;
								variant_interface_SingalLR[1] = 0xD2;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xC8;
								variant_interface_SingalLR[5] = 0xFD;
								variant_interface_SingalLR[6] = 0xBC;
								variant_interface_SingalLR[7] = 0xDC;
								variant_interface_SingalLR[8] = '\0';
								break;
							default:
								break;
								 
							
						}
					}
					else if(_unitSingalCMD.Mainlogic_singalCMD.direction == DIRECTION_LEFT)
					{
						switch(_unitSingalCMD.Mainlogic_singalCMD.num)
						{
							case 1:
								variant_interface_SingalLR[0] = 0xD7;
								variant_interface_SingalLR[1] = 0xF3;
								variant_interface_SingalLR[2] = 0xC1;
								variant_interface_SingalLR[3] = 0xDA;
								variant_interface_SingalLR[4] = 0xBC;
								variant_interface_SingalLR[5] = 0xDC;
								variant_interface_SingalLR[6] = 0x20;
								variant_interface_SingalLR[7] = 0x20;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 2:
								variant_interface_SingalLR[0] = 0xD7;
								variant_interface_SingalLR[1] = 0xF3;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xBC;
								variant_interface_SingalLR[5] = 0xDC;
								variant_interface_SingalLR[6] = 0x20;
								variant_interface_SingalLR[7] = 0x20;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 3:
								variant_interface_SingalLR[0] = 0xD7;
								variant_interface_SingalLR[1] = 0xF3;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xD2;
								variant_interface_SingalLR[5] = 0xBB;
								variant_interface_SingalLR[6] = 0xBC;
								variant_interface_SingalLR[7] = 0xDC;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 4:
								variant_interface_SingalLR[0] = 0xD7;
								variant_interface_SingalLR[1] = 0xF3;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xB6;
								variant_interface_SingalLR[5] = 0xFE;
								variant_interface_SingalLR[6] = 0xBC;
								variant_interface_SingalLR[7] = 0xDC;
								variant_interface_SingalLR[8] = '\0';
								break;
							case 5:
								variant_interface_SingalLR[0] = 0xD7;
								variant_interface_SingalLR[1] = 0xF3;
								variant_interface_SingalLR[2] = 0xB8;
								variant_interface_SingalLR[3] = 0xF4;
								variant_interface_SingalLR[4] = 0xC8;
								variant_interface_SingalLR[5] = 0xFD;
								variant_interface_SingalLR[6] = 0xBC;
								variant_interface_SingalLR[7] = 0xDC;
								variant_interface_SingalLR[8] = '\0';
								break;
							default:
								break;
								 
							
						}									
					}
					variant_Mainlogic_Qreturn = xQueueReceive(Key_Value_ISR_Queue,&variant_Mainlogic_KeyValue,Structset_GlobalSet_Settingtime3_1.Setting_Para*1000);//
					if(variant_Mainlogic_Qreturn == pdFALSE)
					{
						variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case
						_unitSingalCMD.Mainlogic_singalCMD.movetype= SINGAL_CANCEI;//取下
						_unitSingalCMD.Mainlogic_singalCMD.moveCMD= MOVE_NONE;
						xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);	
						State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
						/********************************改变设备状态---极为重要，本任务只有一对**********************************/
						_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
						/********************************改变设备状态---极为重要，本任务只有一对**********************************/
						_variant_Interface_disp.cursor  = 0;
						_variant_Interface_disp.type = TYPE_DEFAULT;
						variant_Mainlogic_Qreturn = 0;
						LED_Management_Fanction(LED_NORMOL);
						xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);						
					}
					else
					{
						State_main_TrumpetStatus = 1;
						
						 switch(variant_Mainlogic_KeyValue)
						{
							case KEY20_VALUE:
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_PITCH_ON;//保证切换成功
								_variant_Interface_disp.cursor =(_variant_Interface_disp.cursor+1)%CURSOR_TYPE_SINGLE_CHOSE_LIMT;
								if(_variant_Interface_disp.cursor == 0)
									_variant_Interface_disp.cursor = _variant_Interface_disp.cursor|0x01;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
                                break;						
							case KEY24_VALUE:
								if(_variant_Interface_disp.cursor ==1)
									_variant_Interface_disp.cursor = CURSOR_TYPE_SINGLE_CHOSE_LIMT-1;
								else 
									_variant_Interface_disp.cursor--;
								
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
								
							case KEY26_VALUE:
								//如果是左邻架，跳转到左隔架;如果是右邻架，跳转到右隔架
								_unitSingalCMD.Mainlogic_singalCMD.num = 2;
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_PITCH_ON;
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
								
							case KEY3_VALUE:

								if((_unitSingalCMD.Mainlogic_singalCMD.direction == DIRECTION_RIGHT)&&(_unitSingalCMD.Mainlogic_singalCMD.num<=5)&&(_unitSingalCMD.Mainlogic_singalCMD.num>=2))
								{
										_unitSingalCMD.Mainlogic_singalCMD.num++;
										_unitSingalCMD.Mainlogic_singalCMD.back_Flag = 0;
								}
								else if((_unitSingalCMD.Mainlogic_singalCMD.direction == DIRECTION_LEFT)&&(_unitSingalCMD.Mainlogic_singalCMD.num>1))
								{
										_unitSingalCMD.Mainlogic_singalCMD.num--;
										_unitSingalCMD.Mainlogic_singalCMD.back_Flag = 1;
								}
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_PITCH_ON;
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
								
							case KEY7_VALUE:

								if((_unitSingalCMD.Mainlogic_singalCMD.direction == DIRECTION_LEFT)&&(_unitSingalCMD.Mainlogic_singalCMD.num<=5)&&(_unitSingalCMD.Mainlogic_singalCMD.num>=2))
								{
										_unitSingalCMD.Mainlogic_singalCMD.num++;
										_unitSingalCMD.Mainlogic_singalCMD.back_Flag = 0;
								}
								else if((_unitSingalCMD.Mainlogic_singalCMD.direction == DIRECTION_RIGHT)&&(_unitSingalCMD.Mainlogic_singalCMD.num>1))
								{
										_unitSingalCMD.Mainlogic_singalCMD.num--;
										_unitSingalCMD.Mainlogic_singalCMD.back_Flag = 1;
								}
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_PITCH_ON;
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
							
							/***********************下面是具体操作***********************/
							
							/********单个支架自动降-移-升控制********/
							case KEY16_VALUE:	//按下启动按键，开始自动自动降-移-升控制

								break;
								
								case KEY12_VALUE:	//取消键
									variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case
									_unitSingalCMD.Mainlogic_singalCMD.movetype= SINGAL_CANCEI;//取下
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD= MOVE_NONE;
									xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);	
									State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
								/********************************改变设备状态---极为重要，本任务只有一对**********************************/
									_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
								/********************************改变设备状态---极为重要，本任务只有一对**********************************/
									variant_Mainlogic_KeyValue = 0;
									_variant_Interface_disp.cursor  = 0;
									
								
									Display_context_show(default_page,12,0,0);
									LED_Management_Fanction(LED_NORMOL);
								break;
								
								/***************首先是KEY28加，KEY27减****************/
							case KEY27_VALUE:							
								if(_variant_Interface_disp.cursor == 1) 
								{
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
									strcpy(variant_interface_SingalMove,"备用动作  ");
								}
								else if(_variant_Interface_disp.cursor == 2) 
								{
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
									strcpy(variant_interface_SingalMove,"备用动作  ");
								}
								else if(_variant_Interface_disp.cursor == 3) 
								{
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
									strcpy(variant_interface_SingalMove,"备用动作 ");
								}
								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;//can发送取消选中命令
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
	
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);				
								break;
								
								
							case KEY28_VALUE:	
								
								if(_variant_Interface_disp.cursor == 1) 
								{
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
									strcpy(variant_interface_SingalMove,"备用动作 ");
								}
								else if(_variant_Interface_disp.cursor == 2) 
								{
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
									strcpy(variant_interface_SingalMove,"备用动作   ");
								}
								else if(_variant_Interface_disp.cursor == 3) 
								{
									_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
									strcpy(variant_interface_SingalMove,"备用动作 ");
								}
								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY21_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_COLUMN_DOWN;
								strcpy(variant_interface_SingalMove,"降立柱 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY32_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_HYDRAULICFRAME_ADVANCING;
								strcpy(variant_interface_SingalMove,"移架 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY30_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_COLUMN_UP ;
								strcpy(variant_interface_SingalMove,"升柱 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY31_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_CHUTE_PUSH;
								strcpy(variant_interface_SingalMove,"推溜 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY17_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_BALANCE_BEAM_STRETCH;
								strcpy(variant_interface_SingalMove,"平衡伸 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY22_VALUE:	
								
//								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_DROP_AD_UP;
//								strcpy(variant_interface_SingalMove,"降移升   ");

//								/**************改变屏显模式****************/
//								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
//								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
//								/*****************************/
//								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
//								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
//								variant_Mainlogic_Qreturn = 0;
//								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
//								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY18_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_BALANCE_BEAM_WITHDRAW;
								strcpy(variant_interface_SingalMove,"平衡收   ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY13_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_SIDE_PLATE_STRETCH;
								strcpy(variant_interface_SingalMove,"伸侧推 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY14_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_SIDE_PLATE_WITHDRAW;
								strcpy(variant_interface_SingalMove,"收侧推 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY9_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_HYDRAULIC_BOTTOM_STRETCH;
								strcpy(variant_interface_SingalMove,"伸底调   ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY10_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_HYDRAULIC_BOTTOM_WITHDRAW;
								strcpy(variant_interface_SingalMove,"收底调   ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY5_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_FUEL_SPRAY;
								strcpy(variant_interface_SingalMove,"喷雾 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY6_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_STANDBY_APPLICATION1;
								strcpy(variant_interface_SingalMove,"备用   ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
   							    Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								
								case KEY11_VALUE:	
								
//								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_FUEL_SPRAY;
//								strcpy(variant_interface_SingalMove,"喷雾 ");

//								/**************改变屏显模式****************/
//								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
//								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
//								/*****************************/
//								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
//								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
//								variant_Mainlogic_Qreturn = 0;
//								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
//								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								case KEY1_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_BACK_WASH2;
								strcpy(variant_interface_SingalMove,"反冲洗1 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
								
								
								case KEY2_VALUE:	
								
								_unitSingalCMD.Mainlogic_singalCMD.moveCMD = MOVE_BACK_WASH1;
								strcpy(variant_interface_SingalMove,"反冲洗2 ");

								/**************改变屏显模式****************/
								_unitSingalCMD.Mainlogic_singalCMD.movetype = SINGAL_MOVE_START;
								_variant_Interface_disp.type = 	TYPE_SINGLE_MOVE;
								/*****************************/
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
								Main_Logic_SingalMove(variant_Mainlogic_KeyValue);
								variant_Mainlogic_Qreturn = 0;
								_variant_Interface_disp.type = 	TYPE_SINGLE_CHOSE;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								
								break;
							
							default:

								break;
						
						}
					}
					break;
			 case MAINLOGIC_GROUP:
					switch(_unitGroupCMD.Mainlogic_groupCMD.direction1)
					{
						case DIRECTION_R_RIGHT:
								variant_interface_Grouptail[0] = (Group_tail/100)+0x30;
								variant_interface_Grouptail[1] = ((Group_tail%100)/10)+0x30;
								variant_interface_Grouptail[2] = (Group_tail%10)+0x30;
								variant_interface_Grouptail[3] = '\0';
								variant_interface_Grouphead[0] = (Group_head/100)+0x30;
								variant_interface_Grouphead[1] = ((Group_head%100)/10)+0x30;
								variant_interface_Grouphead[2] = (Group_head%10)+0x30;
								variant_interface_Grouphead[3] = '\0';
						
								variant_interface_arrows[0] = '-';
								variant_interface_arrows[1] = '-';
								variant_interface_arrows[2] = '>';
								variant_interface_arrows[3] = '\0';

							break;
						
						case DIRECTION_R_LEFT:
							variant_interface_Grouphead[0] = (Group_tail/100)+0x30;
								variant_interface_Grouphead[1] = ((Group_tail%100)/10)+0x30;
								variant_interface_Grouphead[2] = (Group_tail%10)+0x30;
								variant_interface_Grouphead[3] = '\0';
								variant_interface_Grouptail[0] = (Group_head/100)+0x30;
								variant_interface_Grouptail[1] = ((Group_head%100)/10)+0x30;
								variant_interface_Grouptail[2] = (Group_head%10)+0x30;
								variant_interface_Grouptail[3] = '\0';
								variant_interface_arrows[0] = '-';
								variant_interface_arrows[1] = '-';
								variant_interface_arrows[2] = '>';
								variant_interface_arrows[3] = '\0';
							break;
						
						case DIRECTION_L_LEFT:
							variant_interface_Grouphead[0] = (Group_tail/100)+0x30;
								variant_interface_Grouphead[1] = ((Group_tail%100)/10)+0x30;
								variant_interface_Grouphead[2] = (Group_tail%10)+0x30;
								variant_interface_Grouphead[3] = '\0';
								variant_interface_Grouptail[0] = (Group_head/100)+0x30;
								variant_interface_Grouptail[1] = ((Group_head%100)/10)+0x30;
								variant_interface_Grouptail[2] = (Group_head%10)+0x30;
								variant_interface_Grouptail[3] = '\0';
								variant_interface_arrows[0] = '<';
								variant_interface_arrows[1] = '-';
								variant_interface_arrows[2] = '-';
								variant_interface_arrows[3] = '\0';
							break;
						
						case DIRECTION_L_RIGHT:
								variant_interface_Grouptail[0] = (Group_tail/100)+0x30;
								variant_interface_Grouptail[1] = ((Group_tail%100)/10)+0x30;
								variant_interface_Grouptail[2] = (Group_tail%10)+0x30;
								variant_interface_Grouptail[3] = '\0';
								variant_interface_Grouphead[0] = (Group_head/100)+0x30;
								variant_interface_Grouphead[1] = ((Group_head%100)/10)+0x30;
								variant_interface_Grouphead[2] = (Group_head%10)+0x30;
								variant_interface_Grouphead[3] = '\0';
								variant_interface_arrows[0] = '<';
								variant_interface_arrows[1] = '-';
								variant_interface_arrows[2] = '-';
								variant_interface_arrows[3] = '\0';
							break;
						
						default :
							break;
					
					
					
					}
					xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);			 
					variant_Mainlogic_Qreturn = xQueueReceive(Key_Value_ISR_Queue,&variant_Mainlogic_KeyValue,Structset_GlobalSet_Settingtime3_1.Setting_Para*1000);//
					if(variant_Mainlogic_Qreturn == pdFALSE)
					{
						variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case
						memset(&_unitGroupCMD.arrayKEYGroup,0,8);	
						State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
						/********************************改变设备状态---极为重要，本任务只有一对**********************************/
						_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
						/********************************改变设备状态---极为重要，本任务只有一对**********************************/
						_variant_Interface_disp.cursor  = 0;
						_variant_Interface_disp.type = TYPE_DEFAULT;
						variant_Mainlogic_Qreturn = 0;
						LED_Management_Fanction(LED_NORMOL);
						xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);						
					}
					else{
							State_main_TrumpetStatus = 1;
						
							 switch(variant_Mainlogic_KeyValue)								
							{
								case KEY3_VALUE:
								case KEY7_VALUE:
									switch(_unitGroupCMD.Mainlogic_groupCMD.direction1)
									{
										case DIRECTION_R_RIGHT:
											LED_Management_Fanction(LED_GROUPRIGHT);
											_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_RIGHT;
											break;
										case DIRECTION_L_RIGHT:
											LED_Management_Fanction(LED_GROUPRIGHT);
											_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_R_RIGHT;
											break;
										case DIRECTION_L_LEFT:
											LED_Management_Fanction(LED_GROUPLEFT);
											_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_R_LEFT;
											break;
										case DIRECTION_R_LEFT:
											LED_Management_Fanction(LED_GROUPLEFT);
								
											_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
											break;
									}

								
									break;
								case KEY4_VALUE:
									if(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_L_RIGHT)
									{
										_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_R_RIGHT;
									}
									else if(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_R_RIGHT)
									{
										_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_RIGHT;
									}
									break;
								case KEY8_VALUE:
									if(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_R_LEFT)
									{
										_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
									}
									else if(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_L_LEFT)
									{
										_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_R_LEFT;
									}
									break;
								case KEY16_VALUE:
									if(_unitGroupCMD.Mainlogic_groupCMD.type0 == TPYE_GROUP_ALARM)
									{
										xQueueSend(CAN_Send_Queue,&_unitGroupCMD.arrayKEYGroup,0);
//										if((_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_L_LEFT)||(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_R_RIGHT))
//										{
//											_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
//											xQueueSend(CAN_Send_Queue,&_unitGroupCMD.arrayKEYGroup,0);

//										}
									}
									else
									{	
											if(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_R_LEFT)	
											{
												_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
											}
											else if(_unitGroupCMD.Mainlogic_groupCMD.direction1 == DIRECTION_L_RIGHT)
											{												
												_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_R_RIGHT;
											}
											xQueueSend(CAN_Send_Queue,&_unitGroupCMD.arrayKEYGroup,0);	
									}
									Flag_Mainlogic_Groupcontorl = _unitGroupCMD.Mainlogic_groupCMD.direction1&0xf;
									variant_Mainlogic_GroupMovenum = _unitGroupCMD.Mainlogic_groupCMD.gap6+_unitGroupCMD.Mainlogic_groupCMD.movenum3;//有一个需负责的成组
										variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case
										memset(&_unitGroupCMD.arrayKEYGroup,0,8);	
										State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;//按键进入默认界面状态
										/********************************改变设备状态---极为重要，本任务只有一对**********************************/
										_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
										/********************************改变设备状态---极为重要，本任务只有一对**********************************/
										_variant_Interface_disp.cursor  = 0;
										_variant_Interface_disp.type = TYPE_DEFAULT;
										variant_Mainlogic_Qreturn = 0;
										LED_Management_Fanction(LED_NORMOL);
										xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);									
									break;
								case KEY12_VALUE:
										variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case
										memset(&_unitGroupCMD.arrayKEYGroup,0,8);	
										State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
										/********************************改变设备状态---极为重要，本任务只有一对**********************************/
										_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
										/********************************改变设备状态---极为重要，本任务只有一对**********************************/
										_variant_Interface_disp.cursor  = 0;
										_variant_Interface_disp.type = TYPE_DEFAULT;
										variant_Mainlogic_Qreturn = 0;
										LED_Management_Fanction(LED_NORMOL);
										xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);		
									break;
								
								
								case KEY32_VALUE:	
								
									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP_ALARM;
	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_ADVANCING;	
									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Framemove_GroupSeparatedNum24_1.Setting_Para,Structset_Framemove_GroupRange25_1.Setting_Para);
									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
									Group_head =  *(p+2);
									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Framemove_GroupSeparatedNum24_1.Setting_Para;
	//								_variant_Interface_disp.type = 	TYPE_GROUP_LEFT;
									
									variant_interface_move[0] = 0xD2;
									variant_interface_move[1] = 0xC6;
									variant_interface_move[2] = 0xBC;
									variant_interface_move[3] = 0xDC;
									variant_interface_move[4] = '\0';
									variant_interface_move[5] = '\0';
									variant_interface_move[6] = '\0';//汉字移架

									
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
									
								break;
//								case KEY11_VALUE:	//喷雾
//								
//									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
//	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
//									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_SPRAY_OUT;//喷雾	
//									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Mistspray_SeparatedNum8_1.Setting_Para,Structset_Mistspray_Range9_1.Setting_Para);
//									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
//									Group_head =  *(p+2);
//									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
//									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Mistspray_SeparatedNum8_1.Setting_Para;
//								
//									variant_interface_move[0] = 0xC5;
//									variant_interface_move[1] = 0xE7;
//									variant_interface_move[2] = 0xCE;
//									variant_interface_move[3] = 0xED;
//									variant_interface_move[4] = '\0';
//									variant_interface_move[5] = '\0';
//									variant_interface_move[6] = '\0';//汉字喷雾
//								
//									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//								break;
//								case KEY5_VALUE://伸前梁
//									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
//	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
//									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_TELESCOPIC_GIRDER_STRETCH;//伸前梁	
//									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Telescopict_Separatednum5_1.Setting_Para,Structset_Telescopict_Range6_1.Setting_Para);
//									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
//									Group_head =  *(p+2);
//									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
//									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Telescopict_Separatednum5_1.Setting_Para;
//								
//									variant_interface_move[0] = 0xC9;
//									variant_interface_move[1] = 0xEC;
//									variant_interface_move[2] = 0xC7;
//									variant_interface_move[3] = 0xB0;
//									variant_interface_move[4] = 0xC1;
//									variant_interface_move[5] = 0xBA;
//									variant_interface_move[6] = '\0';//汉字移架
//								
//									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//									break;
//								
//								case KEY6_VALUE://收前梁
//									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP_ALARM;
//	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
//									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUPTELESCOPICPLATE_GIRDER_WITHDRAW;//收前梁	
//									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Telescopict_Separatednum5_1.Setting_Para,Structset_Telescopict_Range6_1.Setting_Para);
//									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
//									Group_head =  *(p+2);
//									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
//									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Telescopict_Separatednum5_1.Setting_Para;
//								
//									variant_interface_move[0] = 0xCA;
//									variant_interface_move[1] = 0xD5;
//								    variant_interface_move[2] = 0xC7;
//									variant_interface_move[3] = 0xB0;
//									variant_interface_move[4] = 0xC1;
//									variant_interface_move[5] = 0xBA;
//									variant_interface_move[6] = '\0';//汉字移架
//								
//									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//									break;
									
								case KEY31_VALUE://推溜
									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_CHUTE_PUSH;//推溜	
									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_ChutePush_GroupSeparatedNum6_1.Setting_Para,Structset_ChutePush_GroupRange7_1.Setting_Para);
									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
									Group_head =  *(p+2);
									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_ChutePush_GroupSeparatedNum6_1.Setting_Para;
								
									variant_interface_move[0] = 0xCD;
									variant_interface_move[1] = 0xC6;
									variant_interface_move[2] = 0xC1;
									variant_interface_move[3] = 0xEF;
									variant_interface_move[4] = '\0';
									variant_interface_move[5] = '\0';
									variant_interface_move[6] = '\0';//汉字移架
								
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
									break;
								
//								case KEY10_VALUE://收护帮
//									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
//	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
//									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_PLATE_WITHDRAW;//收护帮	
//									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Telescopict_Separatednum5_1.Setting_Para,Structset_Telescopict_Range6_1.Setting_Para);
//									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
//									Group_head =  *(p+2);
//									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
//									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Telescopict_Separatednum5_1.Setting_Para;
//								
//									variant_interface_move[0] = 0xCA;
//									variant_interface_move[1] = 0xD5;
//									variant_interface_move[2] = 0xBB;
//									variant_interface_move[3] = 0xA4;
//									variant_interface_move[4] = 0xB0;
//									variant_interface_move[5] = 0xEF;
//									variant_interface_move[6] = '\0';//汉字移架
//								
//									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//									break;
								
//								case KEY9_VALUE://伸护帮
//									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
//	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
//									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_PLATE_STRETCH;//伸护帮	
//									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_Telescopict_Separatednum5_1.Setting_Para,Structset_Telescopict_Range6_1.Setting_Para);
//									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
//									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
//									Group_head =  *(p+2);
//									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
//									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_Telescopict_Separatednum5_1.Setting_Para;
//								
//									variant_interface_move[0] = 0xC9;
//									variant_interface_move[1] = 0xEC;
//									variant_interface_move[2] = 0xBB;
//									variant_interface_move[3] = 0xA4;
//									variant_interface_move[4] = 0xB0;
//									variant_interface_move[5] = 0xEF;
//									variant_interface_move[6] = '\0';//汉字移架
//								
//									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
//									break;
								case KEY18_VALUE://拉溜
									
									_unitGroupCMD.Mainlogic_groupCMD.type0 = TPYE_GROUP;
	//								_unitGroupCMD.Mainlogic_groupCMD.direction1 = DIRECTION_L_LEFT;
									_unitGroupCMD.Mainlogic_groupCMD.moveCMD4 = MOVE_GROUP_CHUTE_PULL;//拉溜
									p=Display_Group_Movedirectionjudge(_unitGroupCMD.Mainlogic_groupCMD.direction1,Structset_ChutePull_GroupSeparatedNum5_1.Setting_Para,Structset_ChutePull_GroupRange6_1.Setting_Para);
									_unitGroupCMD.Mainlogic_groupCMD.alarmnum2 = *p;
									_unitGroupCMD.Mainlogic_groupCMD.movenum3 = *p;
									_unitGroupCMD.Mainlogic_groupCMD.limit5 = *(p+1);
									Group_head =  *(p+2);
									Group_tail = _unitGroupCMD.Mainlogic_groupCMD.limit5;
									_unitGroupCMD.Mainlogic_groupCMD.gap6 = Structset_ChutePull_GroupSeparatedNum5_1.Setting_Para;
								
									variant_interface_move[0] = 0xC0;
									variant_interface_move[1] = 0xAD;
									variant_interface_move[2] = 0xC1;
									variant_interface_move[3] = 0xEF;
									variant_interface_move[4] = '\0';
									variant_interface_move[5] = '\0';
									variant_interface_move[6] = '\0';//汉字移架
								
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
									break;
					
								default:
								break;
							}
					}
					break;
			 case MAINLOGIC_SETTING:
					variant_Mainlogic_Qreturn = xQueueReceive(Key_Value_ISR_Queue,&variant_Mainlogic_KeyValue,Structset_GlobalSet_Settingtime3_1.Setting_Para*1000);//
					if(variant_Mainlogic_Qreturn == pdFALSE)
					{
						variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case	
						State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
						/********************************改变设备状态---极为重要，本任务只有一对**********************************/
						_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
						/********************************改变设备状态---极为重要，本任务只有一对**********************************/
						_variant_Interface_disp.cursor  = 0;//光标复位
						_variant_Interface_disp.type = TYPE_DEFAULT;
						variant_Mainlogic_Qreturn = 0;
						if(Flag_Set_Save == 1)
						{
							SaveParaToFlash_WritingFun();
							Flag_Set_Save = 0;
						}
						xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);						
					}
					else
					{
						State_main_TrumpetStatus = 1;

						 switch(variant_Mainlogic_KeyValue)
						{
							case KEY19_VALUE://右键设置选单	
								variant_Mainlogic_KeyValue = 0;
								switch(_variant_Interface_disp.type)
								{

					                case TYPE_SETTING_GLOBAL_SET:
										_variant_Interface_disp.type = TYPE_SETTING_SERVICE;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_SERVICE_LIMT;
										break;
									case TYPE_SETTING_SERVICE:
									 	_variant_Interface_disp.type = TYPE_SETTING_DEFAULT_PARAMETERS;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_DEFAULT_PARAMETERS_LIMT;
										break;
									case TYPE_SETTING_DEFAULT_PARAMETERS:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_ADVANCING;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_ADVANCING_LIMT;
										break;
									case TYPE_SETTING_GROUP_ADVANCING:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_CHUTE_PUSH;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_CHUTE_PUSH_LIMT;
										break;
									case TYPE_SETTING_GROUP_CHUTE_PUSH:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_BALANCE;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_BALANCE_LIMT;
										break;
									case TYPE_SETTING_GROUP_BALANCE:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_TELESCOPIC;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_TELESCOPIC_LIMT;
										break;
									case TYPE_SETTING_GROUP_TELESCOPIC:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_NC;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_NC_LIMT;
										break;
									case TYPE_SETTING_GROUP_NC:
										_variant_Interface_disp.type = TYPE_SETTING_AUTOMATION;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_AUTOMATION_LIMT;
										break;
									case TYPE_SETTING_AUTOMATION:
										_variant_Interface_disp.type = TYPE_SETTING_FAULT;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_FAULT_LIMT;
										break;
									case TYPE_SETTING_FAULT:
										_variant_Interface_disp.type = TYPE_SETTING_GLOBAL_SET;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GLOBAL_SET_LIMT;
										break;
									default:
										break;		
								}
								_variant_Interface_disp.cursor = 1;									
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
							case KEY23_VALUE://左键设置选单		
								variant_Mainlogic_KeyValue = 0;
							    switch(_variant_Interface_disp.type)
								{
					                case TYPE_SETTING_GLOBAL_SET:
										_variant_Interface_disp.type = TYPE_SETTING_FAULT;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_FAULT_LIMT;
										break;
									case TYPE_SETTING_SERVICE:
										variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GLOBAL_SET_LIMT;
										_variant_Interface_disp.type = TYPE_SETTING_GLOBAL_SET;
									 	
										break;
									case TYPE_SETTING_DEFAULT_PARAMETERS:
										_variant_Interface_disp.type = TYPE_SETTING_SERVICE;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_SERVICE_LIMT;
									
										break;
									case TYPE_SETTING_GROUP_ADVANCING:
										_variant_Interface_disp.type = TYPE_SETTING_DEFAULT_PARAMETERS;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_DEFAULT_PARAMETERS_LIMT;
						
										break;
									case TYPE_SETTING_GROUP_CHUTE_PUSH:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_ADVANCING;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_ADVANCING_LIMT;
										
										break;
									case TYPE_SETTING_GROUP_BALANCE:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_CHUTE_PUSH;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_CHUTE_PUSH_LIMT;
										
										break;
									case TYPE_SETTING_GROUP_TELESCOPIC:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_BALANCE;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_BALANCE_LIMT;
										
										break;
									case TYPE_SETTING_GROUP_NC:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_TELESCOPIC;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_TELESCOPIC_LIMT;
										
										break;
									case TYPE_SETTING_AUTOMATION:
										_variant_Interface_disp.type = TYPE_SETTING_GROUP_NC;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_GROUP_NC_LIMT;
										break;
									case TYPE_SETTING_FAULT:
										_variant_Interface_disp.type = TYPE_SETTING_AUTOMATION;
									    variant_Interface_SETTINGcursorLimit = CURSOR_TYPE_SETTING_AUTOMATION_LIMT;
										break;
									default:
										break;		
								}
							    _variant_Interface_disp.cursor = 1;							
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
							case KEY20_VALUE://下键设置选单	
								variant_Mainlogic_KeyValue = 0;	//按键值
							    _variant_Interface_disp.cursor =(_variant_Interface_disp.cursor+1)%variant_Interface_SETTINGcursorLimit;
								if(_variant_Interface_disp.cursor == 0)
									_variant_Interface_disp.cursor = _variant_Interface_disp.cursor|0x01;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
							case KEY24_VALUE://上键设置选单
								if(_variant_Interface_disp.cursor ==1)
									_variant_Interface_disp.cursor = variant_Interface_SETTINGcursorLimit-1;
								else 
									_variant_Interface_disp.cursor--;								
								variant_Mainlogic_KeyValue = 0;	
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
								break;
							case KEY26_VALUE:	//确定
								    if(Flag_Set_Passwd == 0)//没有输入过密码
									{
										LED_Management_Fanction(LED_SETTING_NUM);
										Flag_Set_Passwd = PasswdInput_Set_Fun();
										if(Flag_Set_Passwd)
											Set_Prar_Change(_variant_Interface_disp.type,_variant_Interface_disp.cursor);
										LED_Management_Fanction(LED_NORMOL);
									}
									else//输入过正确的密码，进入参数设置界面
									{
										Set_Prar_Change(_variant_Interface_disp.type,_variant_Interface_disp.cursor);
									}
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
									break;
								
							case KEY12_VALUE:	//取消键							
									variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case	
									State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
									/********************************改变设备状态---极为重要，本任务只有一对**********************************/
									_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
									/********************************改变设备状态---极为重要，本任务只有一对**********************************/
									_variant_Interface_disp.cursor  = 0;//光标复位
									variant_Mainlogic_Qreturn = 0;
									LED_Management_Fanction(LED_NORMOL);
									_variant_Interface_disp.type = TYPE_DEFAULT;
									if(Flag_Set_Save == 1)
									{
										SaveParaToFlash_WritingFun();
										Flag_Set_Save = 0;
									}
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);	
								break;
							default:
								break;
						}
						
					}
					break;
					case MAINLOGIC_WORKINGCONDITION:
						variant_Mainlogic_Qreturn = xQueueReceive(Key_Value_ISR_Queue,&variant_Mainlogic_KeyValue,Structset_GlobalSet_Settingtime3_1.Setting_Para*300);//
						if(variant_Mainlogic_Qreturn == pdFALSE)
						{
							if(Count_Mainlogic_workingCondition<4)
							{
								Count_Mainlogic_workingCondition++;
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
							}
							else if(Count_Mainlogic_workingCondition>=4)
							{
								Count_Mainlogic_workingCondition = 0;
								variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case	
								State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
								/********************************改变设备状态---极为重要，本任务只有一对**********************************/
								_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
								/********************************改变设备状态---极为重要，本任务只有一对**********************************/
								_variant_Interface_disp.cursor  = 0;//光标复位
								_variant_Interface_disp.type = TYPE_DEFAULT;
								variant_Mainlogic_Qreturn = 0;
								LED_Management_Fanction(LED_NORMOL);
								xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);		
							}							
						}
						else
						{
								Count_Mainlogic_workingCondition = 0;
							 State_main_TrumpetStatus = 1;
							 switch(variant_Mainlogic_KeyValue)
							 {
								 case KEY12_VALUE:
									 Count_Mainlogic_workingCondition = 0;
									variant_Mainlogic_KeyValue = 0;//不要进入下面的switch中的任何一个case	
									State_Mainlogic_pKEYstu = MAINLOGIC_DWFAULT;
									/********************************改变设备状态---极为重要，本任务只有一对**********************************/
									_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
									/********************************改变设备状态---极为重要，本任务只有一对**********************************/
									_variant_Interface_disp.cursor  = 0;//光标复位
									_variant_Interface_disp.type = TYPE_DEFAULT;
									variant_Mainlogic_Qreturn = 0;
									LED_Management_Fanction(LED_NORMOL);
									xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);	
									break;
								 
								 case KEY20_VALUE:
									 if(_variant_Interface_disp.type == TYPE_WORKINGCONDITION_WORKINGCONDITION1)
									 {
										_variant_Interface_disp.type=TYPE_WORKINGCONDITION_WORKINGCONDITION2;									  
									 }
									 else if(_variant_Interface_disp.type == TYPE_WORKINGCONDITION_WORKINGCONDITION2)
									 {
										_variant_Interface_disp.type=TYPE_WORKINGCONDITION_WORKINGCONDITION1;								 
									 }
									 xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
									 break;
								 
								 case KEY24_VALUE:
									 if(_variant_Interface_disp.type == TYPE_WORKINGCONDITION_WORKINGCONDITION1)
									 {
										_variant_Interface_disp.type=TYPE_WORKINGCONDITION_WORKINGCONDITION2;									  
									 }
									 else if(_variant_Interface_disp.type == TYPE_WORKINGCONDITION_WORKINGCONDITION2)
									 {
										_variant_Interface_disp.type=TYPE_WORKINGCONDITION_WORKINGCONDITION1;								 
									 }
									 xQueueSend(Interface_Mian_Logic_Queue,&const_Mainlogic_InterfaceFlag,0);
									 break;
								 
								 case KEY26_VALUE:
									 array_Angel_send[1] = 0x77;
									 usart7_niming_report(3);
									 break;
								
							 }
						} 
						break;
			 default:

				 break;			
		 
		 }	

}


void Set_Prar_Change(uint8_t Setting_Crosswise,uint8_t Setting_Lengthways)
{

u8 array_Follow[8];	
	u8 array_RESET[8];
	switch(Setting_Crosswise)
	{

		case TYPE_SETTING_GLOBAL_SET:
				switch(Setting_Lengthways)
				{
					case 1:
						Structset_GlobalSet_MaxDeviceNum1_1.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_MaxDeviceNum1_1.veiw_type,Structset_GlobalSet_MaxDeviceNum1_1.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_MaxDeviceNum1_1.Setting_Para,Structset_GlobalSet_MaxDeviceNum1_1.veiw_type,Structset_GlobalSet_MaxDeviceNum1_1.Setting_Char);	
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x01;
						}
					break;
						
					case 2:
						Structset_GlobalSet_minDeviceNum2_1.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_minDeviceNum2_1.veiw_type,Structset_GlobalSet_minDeviceNum2_1.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_minDeviceNum2_1.Setting_Para,Structset_GlobalSet_minDeviceNum2_1.veiw_type,Structset_GlobalSet_minDeviceNum2_1.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x02;
						}					
					break;
						
					case 3:
						Structset_GlobalSet_Settingtime3_1.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_Settingtime3_1.veiw_type,Structset_GlobalSet_Settingtime3_1.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_Settingtime3_1.Setting_Para,Structset_GlobalSet_Settingtime3_1.veiw_type,Structset_GlobalSet_Settingtime3_1.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x03;
						}					
					break;
			
					case 4:
						Structset_GlobalSet_Increase4_4.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_Increase4_4.veiw_type,Structset_GlobalSet_Increase4_4.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_Increase4_4.Setting_Para,Structset_GlobalSet_Increase4_4.veiw_type,Structset_GlobalSet_Increase4_4.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x04;
						}					
					break;
						
			
					case 5:
						Structset_GlobalSet_Conswitch5_2.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_Conswitch5_2.veiw_type,Structset_GlobalSet_Conswitch5_2.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_Conswitch5_2.Setting_Para,Structset_GlobalSet_Conswitch5_2.veiw_type,Structset_GlobalSet_Conswitch5_2.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x05;
						}					
					break;
					
					case 6:
						Structset_GlobalSet_Conposition6_1.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_Conposition6_1.veiw_type,Structset_GlobalSet_Conposition6_1.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_Conposition6_1.Setting_Para,Structset_GlobalSet_Conposition6_1.veiw_type,Structset_GlobalSet_Conposition6_1.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x06;
						}
					break;
						
					case 7:
						Structset_GlobalSet_PositionUpload7_2.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_PositionUpload7_2.veiw_type,Structset_GlobalSet_PositionUpload7_2.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_PositionUpload7_2.Setting_Para,Structset_GlobalSet_PositionUpload7_2.veiw_type,Structset_GlobalSet_PositionUpload7_2.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x07;
						}
					break;
					
					case 8:
						Structset_GlobalSet_Adjacentcheak8_2.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_Adjacentcheak8_2.veiw_type,Structset_GlobalSet_Adjacentcheak8_2.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_Adjacentcheak8_2.Setting_Para,Structset_GlobalSet_Adjacentcheak8_2.veiw_type,Structset_GlobalSet_Adjacentcheak8_2.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x08;
						}
					break;
					
					case 9:
						Structset_GlobalSet_AdjacentWarring9_2.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_AdjacentWarring9_2.veiw_type,Structset_GlobalSet_AdjacentWarring9_2.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_AdjacentWarring9_2.Setting_Para,Structset_GlobalSet_AdjacentWarring9_2.veiw_type,Structset_GlobalSet_AdjacentWarring9_2.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x09;
						}
					break;
					
					case 10:
						Structset_GlobalSet_Adjacenttime10_1.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_Adjacenttime10_1.veiw_type,Structset_GlobalSet_Adjacenttime10_1.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_Adjacenttime10_1.Setting_Para,Structset_GlobalSet_Adjacenttime10_1.veiw_type,Structset_GlobalSet_Adjacenttime10_1.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x0a;
						}
					break;
					
					case 11:
						Structset_GlobalSet_BUSworkcheak11_2.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_BUSworkcheak11_2.veiw_type,Structset_GlobalSet_BUSworkcheak11_2.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_BUSworkcheak11_2.Setting_Para,Structset_GlobalSet_BUSworkcheak11_2.veiw_type,Structset_GlobalSet_BUSworkcheak11_2.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x0b;
						}
					break;
					
					case 12:
						Structset_GlobalSet_BUSreport12_2.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_BUSreport12_2.veiw_type,Structset_GlobalSet_BUSreport12_2.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_BUSreport12_2.Setting_Para,Structset_GlobalSet_BUSreport12_2.veiw_type,Structset_GlobalSet_BUSreport12_2.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x0c;
						}
					break;
					
					case 13:
						Structset_GlobalSet_BUScheakTime13_1.Setting_Para=Set_Prar_Change_Input(Structset_GlobalSet_BUScheakTime13_1.veiw_type,Structset_GlobalSet_BUScheakTime13_1.Setting_Para);
						Set_Para_to_char(Structset_GlobalSet_BUScheakTime13_1.Setting_Para,Structset_GlobalSet_BUScheakTime13_1.veiw_type,Structset_GlobalSet_BUScheakTime13_1.Setting_Char);
						if(Flag_Set_Save == 1)
						{
							variant_Parameter_Globalset = 0x0d;
						}
					break;
					

					default:
					break;										
				}
				if(Flag_Set_Save == 1)
				{
					Structset_GlobalSet_SynchronizationFun_S();
				}
			break;
			
			case TYPE_SETTING_SERVICE:

				switch(Setting_Lengthways)
				{

					case 1:
						Structset_Service_DeviceNum1_1.Setting_Para=Set_Prar_Change_Input(Structset_Service_DeviceNum1_1.veiw_type,Structset_Service_DeviceNum1_1.Setting_Para);
						Set_Para_to_char(Structset_Service_DeviceNum1_1.Setting_Para,Structset_Service_DeviceNum1_1.veiw_type,Structset_Service_DeviceNum1_1.Setting_Char);
						CAN_Config();
					break;
					case 2:
						Structset_Service_Adjacentcorrelation2_2.Setting_Para=Set_Prar_Change_Input(Structset_Service_Adjacentcorrelation2_2.veiw_type,Structset_Service_Adjacentcorrelation2_2.Setting_Para);
						Set_Para_to_char(Structset_Service_Adjacentcorrelation2_2.Setting_Para,Structset_Service_Adjacentcorrelation2_2.veiw_type,Structset_Service_Adjacentcorrelation2_2.Setting_Char);
					break;
					case 3:

					break;
					case 4:
							Parameter_delivery_S();
					break;
					case 5:						
							Serial_Number_delivery_S();
					break;
					case 6:
							array_RESET[0] = TPYE_RESET;
							CANx_SendNormalData(&RIGHT_CAN,0x10,array_RESET,1);		
					break;
					case 7:						
							array_RESET[0] = TPYE_RESET;
							CANx_SendNormalData(&LEFT_CAN,0x10,array_RESET,1);		
					break;
					
					default:
					break;	
				}
			break;

			case TYPE_SETTING_DEFAULT_PARAMETERS:
				switch(Setting_Lengthways)
				{
					case 1:
						Structset_Default_Pressurecheak1_2.Setting_Para=Set_Prar_Change_Input(Structset_Default_Pressurecheak1_2.veiw_type,Structset_Default_Pressurecheak1_2.Setting_Para);
						Set_Para_to_char(Structset_Default_Pressurecheak1_2.Setting_Para,Structset_Default_Pressurecheak1_2.veiw_type,Structset_Default_Pressurecheak1_2.Setting_Char);
					break;
					case 2:
						Structset_Default_Distancecheak2_2.Setting_Para=Set_Prar_Change_Input(Structset_Default_Distancecheak2_2.veiw_type,Structset_Default_Distancecheak2_2.Setting_Para);
						Set_Para_to_char(Structset_Default_Distancecheak2_2.Setting_Para,Structset_Default_Distancecheak2_2.veiw_type,Structset_Default_Distancecheak2_2.Setting_Char);
					break;
					case 3:
						Structset_Default_Pushtime3_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_Pushtime3_1.veiw_type,Structset_Default_Pushtime3_1.Setting_Para);
						Set_Para_to_char(Structset_Default_Pushtime3_1.Setting_Para,Structset_Default_Pushtime3_1.veiw_type,Structset_Default_Pushtime3_1.Setting_Char);
					break;
					case 4:
						Structset16_Default_Pushdistance4_1.Setting_Para=Set_Prar_Change_Input(Structset16_Default_Pushdistance4_1.veiw_type,Structset16_Default_Pushdistance4_1.Setting_Para);
						Set_Para_to_char16(Structset16_Default_Pushdistance4_1.Setting_Para,Structset16_Default_Pushdistance4_1.veiw_type,Structset16_Default_Pushdistance4_1.Setting_Char);
					break;
					case 5:
						Structset_Default_PressureLowlimt5_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_PressureLowlimt5_1.veiw_type,Structset_Default_PressureLowlimt5_1.Setting_Para);
						Set_Para_to_char(Structset_Default_PressureLowlimt5_1.Setting_Para,Structset_Default_PressureLowlimt5_1.veiw_type,Structset_Default_PressureLowlimt5_1.Setting_Char);
					break;
					case 6:
						Structset_Default_PressureHighlimt6_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_PressureHighlimt6_1.veiw_type,Structset_Default_PressureHighlimt6_1.Setting_Para);
						Set_Para_to_char(Structset_Default_PressureHighlimt6_1.Setting_Para,Structset_Default_PressureHighlimt6_1.veiw_type,Structset_Default_PressureHighlimt6_1.Setting_Char);
					break;
					case 7:
						Structset_Default_aimPressure7_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_aimPressure7_1.veiw_type,Structset_Default_aimPressure7_1.Setting_Para);
						Set_Para_to_char(Structset_Default_aimPressure7_1.Setting_Para,Structset_Default_aimPressure7_1.veiw_type,Structset_Default_aimPressure7_1.Setting_Char);
					break;
					case 8:
						Structset_Default_StepuppressureTime8_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_StepuppressureTime8_1.veiw_type,Structset_Default_StepuppressureTime8_1.Setting_Para);
						Set_Para_to_char(Structset_Default_StepuppressureTime8_1.Setting_Para,Structset_Default_StepuppressureTime8_1.veiw_type,Structset_Default_StepuppressureTime8_1.Setting_Char);
					break;
					case 9:
						Structset_Default_StepuppressureGap9_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_StepuppressureGap9_1.veiw_type,Structset_Default_StepuppressureGap9_1.Setting_Para);
						Set_Para_to_char(Structset_Default_StepuppressureGap9_1.Setting_Para,Structset_Default_StepuppressureGap9_1.veiw_type,Structset_Default_StepuppressureGap9_1.Setting_Char);
					break;
					case 10:
						Structset_Default_Stepuppressuretimes10_1.Setting_Para=Set_Prar_Change_Input(Structset_Default_Stepuppressuretimes10_1.veiw_type,Structset_Default_Stepuppressuretimes10_1.Setting_Para);
						Set_Para_to_char(Structset_Default_Stepuppressuretimes10_1.Setting_Para,Structset_Default_Stepuppressuretimes10_1.veiw_type,Structset_Default_Stepuppressuretimes10_1.Setting_Char);
					break;
					
					default:
					break;	
				}
			break;
	/***************移架*****************/

	
			case TYPE_SETTING_GROUP_ADVANCING:
				switch(Setting_Lengthways)
				{
					case 1:
						Structset_Framemove_Forbiden1_3.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_Forbiden1_3.veiw_type,Structset_Framemove_Forbiden1_3.Setting_Para);
						Set_Para_to_char(Structset_Framemove_Forbiden1_3.Setting_Para,Structset_Framemove_Forbiden1_3.veiw_type,Structset_Framemove_Forbiden1_3.Setting_Char);
					break;
					case 2:
						Structset_Framemove_Alarm2_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_Alarm2_2.veiw_type,Structset_Framemove_Alarm2_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_Alarm2_2.Setting_Para,Structset_Framemove_Alarm2_2.veiw_type,Structset_Framemove_Alarm2_2.Setting_Char);
					break;
					case 3:
						Structset_Framemove_AlarmTime3_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_AlarmTime3_1.veiw_type,Structset_Framemove_AlarmTime3_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_AlarmTime3_1.Setting_Para,Structset_Framemove_AlarmTime3_1.veiw_type,Structset_Framemove_AlarmTime3_1.Setting_Char);
					break;
					case 4:
						Structset_Framemove_movePressure4_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_movePressure4_1.veiw_type,Structset_Framemove_movePressure4_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_movePressure4_1.Setting_Para,Structset_Framemove_movePressure4_1.veiw_type,Structset_Framemove_movePressure4_1.Setting_Char);
					break;
					case 5:
						Structset_Framemove_TransPressure5_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_TransPressure5_1.veiw_type,Structset_Framemove_TransPressure5_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_TransPressure5_1.Setting_Para,Structset_Framemove_TransPressure5_1.veiw_type,Structset_Framemove_TransPressure5_1.Setting_Char);
					break;
					case 6:
						Structset_Framemove_SupportPressure6_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_SupportPressure6_1.veiw_type,Structset_Framemove_SupportPressure6_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_SupportPressure6_1.Setting_Para,Structset_Framemove_SupportPressure6_1.veiw_type,Structset_Framemove_SupportPressure6_1.Setting_Char);
					break;
					case 7:
						Structset_Framemove_TelescopictMove7_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_TelescopictMove7_2.veiw_type,Structset_Framemove_TelescopictMove7_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_TelescopictMove7_2.Setting_Para,Structset_Framemove_TelescopictMove7_2.veiw_type,Structset_Framemove_TelescopictMove7_2.Setting_Char);
					break;
					case 8:
						Structset_Framemove_PlateMove8_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_PlateMove8_2.veiw_type,Structset_Framemove_PlateMove8_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_PlateMove8_2.Setting_Para,Structset_Framemove_PlateMove8_2.veiw_type,Structset_Framemove_PlateMove8_2.Setting_Char);
					break;
					case 9:
						Structset_Framemove_ColumndownDelay9_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_ColumndownDelay9_1.veiw_type,Structset_Framemove_ColumndownDelay9_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_ColumndownDelay9_1.Setting_Para,Structset_Framemove_ColumndownDelay9_1.veiw_type,Structset_Framemove_ColumndownDelay9_1.Setting_Char);
					break;
					case 10:
						Structset_Framemove_ColumndownTime10_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_ColumndownTime10_1.veiw_type,Structset_Framemove_ColumndownTime10_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_ColumndownTime10_1.Setting_Para,Structset_Framemove_ColumndownTime10_1.veiw_type,Structset_Framemove_ColumndownTime10_1.Setting_Char);
					break;
					
					case 11:
						Structset_Framemove_ColumndownBalanceaction11_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_ColumndownBalanceaction11_2.veiw_type,Structset_Framemove_ColumndownBalanceaction11_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_ColumndownBalanceaction11_2.Setting_Para,Structset_Framemove_ColumndownBalanceaction11_2.veiw_type,Structset_Framemove_ColumndownBalanceaction11_2.Setting_Char);
					break;
					case 12:
						Structset_Framemove_Columndownsideplate12_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_Columndownsideplate12_2.veiw_type,Structset_Framemove_Columndownsideplate12_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_Columndownsideplate12_2.Setting_Para,Structset_Framemove_Columndownsideplate12_2.veiw_type,Structset_Framemove_Columndownsideplate12_2.Setting_Char);
					break;
					case 13:
						Structset_Framemove_DcolumnAgainFrequency13_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_DcolumnAgainFrequency13_1.veiw_type,Structset_Framemove_DcolumnAgainFrequency13_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_DcolumnAgainFrequency13_1.Setting_Para,Structset_Framemove_DcolumnAgainFrequency13_1.veiw_type,Structset_Framemove_DcolumnAgainFrequency13_1.Setting_Char);
					break;
					case 14:
						Structset_Framemove_BmttomUP14_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_BmttomUP14_2.veiw_type,Structset_Framemove_BmttomUP14_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_BmttomUP14_2.Setting_Para,Structset_Framemove_BmttomUP14_2.veiw_type,Structset_Framemove_BmttomUP14_2.Setting_Char);
					break;
					case 15:
						Structset16_Framemove_Advancaim15_1.Setting_Para=Set_Prar_Change_Input(Structset16_Framemove_Advancaim15_1.veiw_type,Structset16_Framemove_Advancaim15_1.Setting_Para);
						Set_Para_to_char16(Structset16_Framemove_Advancaim15_1.Setting_Para,Structset16_Framemove_Advancaim15_1.veiw_type,Structset16_Framemove_Advancaim15_1.Setting_Char);
					break;
					case 16:
						Structset_Framemove_AdvancTime16_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_AdvancTime16_1.veiw_type,Structset_Framemove_AdvancTime16_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_AdvancTime16_1.Setting_Para,Structset_Framemove_AdvancTime16_1.veiw_type,Structset_Framemove_AdvancTime16_1.Setting_Char);
					break;
					case 17:
						Structset_Framemove_Advancsideplate17_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_Advancsideplate17_2.veiw_type,Structset_Framemove_Advancsideplate17_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_Advancsideplate17_2.Setting_Para,Structset_Framemove_Advancsideplate17_2.veiw_type,Structset_Framemove_Advancsideplate17_2.Setting_Char);
					break;
					case 18:
						Structset_Framemove_AdvancagainTime18_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_AdvancagainTime18_1.veiw_type,Structset_Framemove_AdvancagainTime18_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_AdvancagainTime18_1.Setting_Para,Structset_Framemove_AdvancagainTime18_1.veiw_type,Structset_Framemove_AdvancagainTime18_1.Setting_Char);
					break;
					case 19:
						Structset_Framemove_ColumnUPtime19_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_ColumnUPtime19_1.veiw_type,Structset_Framemove_ColumnUPtime19_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_ColumnUPtime19_1.Setting_Para,Structset_Framemove_ColumnUPtime19_1.veiw_type,Structset_Framemove_ColumnUPtime19_1.Setting_Char);
					break;
					case 20:
						Structset_Framemove_ColumnUPBalanceaction20_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_ColumnUPBalanceaction20_2.veiw_type,Structset_Framemove_ColumnUPBalanceaction20_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_ColumnUPBalanceaction20_2.Setting_Para,Structset_Framemove_ColumnUPBalanceaction20_2.veiw_type,Structset_Framemove_ColumnUPBalanceaction20_2.Setting_Char);
					break;
					
					case 21:
						Structset_Framemove_ColumnUPsideplatew21_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_ColumnUPsideplatew21_2.veiw_type,Structset_Framemove_ColumnUPsideplatew21_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_ColumnUPsideplatew21_2.Setting_Para,Structset_Framemove_ColumnUPsideplatew21_2.veiw_type,Structset_Framemove_ColumnUPsideplatew21_2.Setting_Char);
					break;
					case 22:
						Structset_Framemove_Spray22_2.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_Spray22_2.veiw_type,Structset_Framemove_Spray22_2.Setting_Para);
						Set_Para_to_char(Structset_Framemove_Spray22_2.Setting_Para,Structset_Framemove_Spray22_2.veiw_type,Structset_Framemove_Spray22_2.Setting_Char);
					break;
					case 23:
						Structset_Framemove_SprayTime23_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_SprayTime23_1.veiw_type,Structset_Framemove_SprayTime23_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_SprayTime23_1.Setting_Para,Structset_Framemove_SprayTime23_1.veiw_type,Structset_Framemove_SprayTime23_1.Setting_Char);
					break;
					case 24:
						Structset_Framemove_GroupSeparatedNum24_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_GroupSeparatedNum24_1.veiw_type,Structset_Framemove_GroupSeparatedNum24_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_GroupSeparatedNum24_1.Setting_Para,Structset_Framemove_GroupSeparatedNum24_1.veiw_type,Structset_Framemove_GroupSeparatedNum24_1.Setting_Char);
					break;
					case 25:
						Structset_Framemove_GroupRange25_1.Setting_Para=Set_Prar_Change_Input(Structset_Framemove_GroupRange25_1.veiw_type,Structset_Framemove_GroupRange25_1.Setting_Para);
						Set_Para_to_char(Structset_Framemove_GroupRange25_1.Setting_Para,Structset_Framemove_GroupRange25_1.veiw_type,Structset_Framemove_GroupRange25_1.Setting_Char);
					break;
					
					default:
					break;	
				
				}
			break;

			case TYPE_SETTING_GROUP_CHUTE_PUSH:
				switch(Setting_Lengthways)
				{
					case 1:
						Structset_ChutePush_Forbiden1_3.Setting_Para=Set_Prar_Change_Input(Structset_ChutePush_Forbiden1_3.veiw_type,Structset_ChutePush_Forbiden1_3.Setting_Para);
						Set_Para_to_char(Structset_ChutePush_Forbiden1_3.Setting_Para,Structset_ChutePush_Forbiden1_3.veiw_type,Structset_ChutePush_Forbiden1_3.Setting_Char);
					break;
					case 2:
						Structset_ChutePush_time2_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePush_time2_1.veiw_type,Structset_ChutePush_time2_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePush_time2_1.Setting_Para,Structset_ChutePush_time2_1.veiw_type,Structset_ChutePush_time2_1.Setting_Char);
					break;
					case 3:
						Structset16_ChutePush_aimDistance3_1.Setting_Para=Set_Prar_Change_Input(Structset16_ChutePush_aimDistance3_1.veiw_type,Structset16_ChutePush_aimDistance3_1.Setting_Para);
						Set_Para_to_char16(Structset16_ChutePush_aimDistance3_1.Setting_Para,Structset16_ChutePush_aimDistance3_1.veiw_type,Structset16_ChutePush_aimDistance3_1.Setting_Char);
					break;
					case 4:
						Structset_ChutePush_Pressure4_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePush_Pressure4_1.veiw_type,Structset_ChutePush_Pressure4_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePush_Pressure4_1.Setting_Para,Structset_ChutePush_Pressure4_1.veiw_type,Structset_ChutePush_Pressure4_1.Setting_Char);
					break;
					case 5:
						Structset16_ChutePush_Maximumdifference5_1.Setting_Para=Set_Prar_Change_Input(Structset16_ChutePush_Maximumdifference5_1.veiw_type,Structset16_ChutePush_Maximumdifference5_1.Setting_Para);
						Set_Para_to_char16(Structset16_ChutePush_Maximumdifference5_1.Setting_Para,Structset16_ChutePush_Maximumdifference5_1.veiw_type,Structset16_ChutePush_Maximumdifference5_1.Setting_Char);
					break;
					case 6:
						Structset_ChutePush_GroupSeparatedNum6_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePush_GroupSeparatedNum6_1.veiw_type,Structset_ChutePush_GroupSeparatedNum6_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePush_GroupSeparatedNum6_1.Setting_Para,Structset_ChutePush_GroupSeparatedNum6_1.veiw_type,Structset_ChutePush_GroupSeparatedNum6_1.Setting_Char);
					break;
					case 7:
						Structset_ChutePush_GroupRange7_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePush_GroupRange7_1.veiw_type,Structset_ChutePush_GroupRange7_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePush_GroupRange7_1.Setting_Para,Structset_ChutePush_GroupRange7_1.veiw_type,Structset_ChutePush_GroupRange7_1.Setting_Char);
					break;
					
					default:
					break;	
				}
			break;

			case TYPE_SETTING_GROUP_BALANCE:
				switch(Setting_Lengthways)
				{
					case 1:
						Structset_ChutePull_Forbiden1_3.Setting_Para=Set_Prar_Change_Input(Structset_ChutePull_Forbiden1_3.veiw_type,Structset_ChutePull_Forbiden1_3.Setting_Para);
						Set_Para_to_char(Structset_ChutePull_Forbiden1_3.Setting_Para,Structset_ChutePull_Forbiden1_3.veiw_type,Structset_ChutePull_Forbiden1_3.Setting_Char);
					break;
					case 2:
						Structset_ChutePull_time2_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePull_time2_1.veiw_type,Structset_ChutePull_time2_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePull_time2_1.Setting_Para,Structset_ChutePull_time2_1.veiw_type,Structset_ChutePull_time2_1.Setting_Char);
					break;
					case 3:
						Structset_ChutePull_aimDistance3_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePull_aimDistance3_1.veiw_type,Structset_ChutePull_aimDistance3_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePull_aimDistance3_1.Setting_Para,Structset_ChutePull_aimDistance3_1.veiw_type,Structset_ChutePull_aimDistance3_1.Setting_Char);
					break;
					case 4:
						Structset_ChutePull_Pressure4_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePull_Pressure4_1.veiw_type,Structset_ChutePull_Pressure4_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePull_Pressure4_1.Setting_Para,Structset_ChutePull_Pressure4_1.veiw_type,Structset_ChutePull_Pressure4_1.Setting_Char);
					break;
					case 5:
						Structset_ChutePull_GroupSeparatedNum5_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePull_GroupSeparatedNum5_1.veiw_type,Structset_ChutePull_GroupSeparatedNum5_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePull_GroupSeparatedNum5_1.Setting_Para,Structset_ChutePull_GroupSeparatedNum5_1.veiw_type,Structset_ChutePull_GroupSeparatedNum5_1.Setting_Char);
					break;
					case 6:
						Structset_ChutePull_GroupRange6_1.Setting_Para=Set_Prar_Change_Input(Structset_ChutePull_GroupRange6_1.veiw_type,Structset_ChutePull_GroupRange6_1.Setting_Para);
						Set_Para_to_char(Structset_ChutePull_GroupRange6_1.Setting_Para,Structset_ChutePull_GroupRange6_1.veiw_type,Structset_ChutePull_GroupRange6_1.Setting_Char);
					break;
					case 7:
						Structset_Mistspray_Forbiden7_3.Setting_Para=Set_Prar_Change_Input(Structset_Mistspray_Forbiden7_3.veiw_type,Structset_Mistspray_Forbiden7_3.Setting_Para);
						Set_Para_to_char(Structset_Mistspray_Forbiden7_3.Setting_Para,Structset_Mistspray_Forbiden7_3.veiw_type,Structset_Mistspray_Forbiden7_3.Setting_Char);
					break;
					case 8:
						Structset_Mistspray_SeparatedNum8_1.Setting_Para=Set_Prar_Change_Input(Structset_Mistspray_SeparatedNum8_1.veiw_type,Structset_Mistspray_SeparatedNum8_1.Setting_Para);
						Set_Para_to_char(Structset_Mistspray_SeparatedNum8_1.Setting_Para,Structset_Mistspray_SeparatedNum8_1.veiw_type,Structset_Mistspray_SeparatedNum8_1.Setting_Char);
					break;
					case 9:
						Structset_Mistspray_Range9_1.Setting_Para=Set_Prar_Change_Input(Structset_Mistspray_Range9_1.veiw_type,Structset_Mistspray_Range9_1.Setting_Para);
						Set_Para_to_char(Structset_Mistspray_Range9_1.Setting_Para,Structset_Mistspray_Range9_1.veiw_type,Structset_Mistspray_Range9_1.Setting_Char);
					break;
					case 10:
						Structset_Mistspray_Time10_1.Setting_Para=Set_Prar_Change_Input(Structset_Mistspray_Time10_1.veiw_type,Structset_Mistspray_Time10_1.Setting_Para);
						Set_Para_to_char(Structset_Mistspray_Time10_1.Setting_Para,Structset_Mistspray_Time10_1.veiw_type,Structset_Mistspray_Time10_1.Setting_Char);
					break;
					
					default:
					break;	
				}

			break;
			case TYPE_SETTING_GROUP_TELESCOPIC:
				switch(Setting_Lengthways)
				{
					case 1:
						Structset_Telescopict_Forbiden1_3.Setting_Para=Set_Prar_Change_Input(Structset_Telescopict_Forbiden1_3.veiw_type,Structset_Telescopict_Forbiden1_3.Setting_Para);
						Set_Para_to_char(Structset_Telescopict_Forbiden1_3.Setting_Para,Structset_Telescopict_Forbiden1_3.veiw_type,Structset_Telescopict_Forbiden1_3.Setting_Char);
					break;
					case 2:
						Structset_Telescopict_StretchTime2_1.Setting_Para=Set_Prar_Change_Input(Structset_Telescopict_StretchTime2_1.veiw_type,Structset_Telescopict_StretchTime2_1.Setting_Para);
						Set_Para_to_char(Structset_Telescopict_StretchTime2_1.Setting_Para,Structset_Telescopict_StretchTime2_1.veiw_type,Structset_Telescopict_StretchTime2_1.Setting_Char);
					break;
					case 3:
						Structset_Telescopict_ShrinkTime3_1.Setting_Para=Set_Prar_Change_Input(Structset_Telescopict_ShrinkTime3_1.veiw_type,Structset_Telescopict_ShrinkTime3_1.Setting_Para);
						Set_Para_to_char(Structset_Telescopict_ShrinkTime3_1.Setting_Para,Structset_Telescopict_ShrinkTime3_1.veiw_type,Structset_Telescopict_ShrinkTime3_1.Setting_Char);
					break;
					case 4:
						Structset_Telescopict_DcolumnTime4_1.Setting_Para=Set_Prar_Change_Input(Structset_Telescopict_DcolumnTime4_1.veiw_type,Structset_Telescopict_DcolumnTime4_1.Setting_Para);
						Set_Para_to_char(Structset_Telescopict_DcolumnTime4_1.Setting_Para,Structset_Telescopict_DcolumnTime4_1.veiw_type,Structset_Telescopict_DcolumnTime4_1.Setting_Char);
					break;
					case 5:
						Structset_Telescopict_Separatednum5_1.Setting_Para=Set_Prar_Change_Input(Structset_Telescopict_Separatednum5_1.veiw_type,Structset_Telescopict_Separatednum5_1.Setting_Para);
						Set_Para_to_char(Structset_Telescopict_Separatednum5_1.Setting_Para,Structset_Telescopict_Separatednum5_1.veiw_type,Structset_Telescopict_Separatednum5_1.Setting_Char);
					break;
					case 6:
						Structset_Telescopict_Range6_1.Setting_Para=Set_Prar_Change_Input(Structset_Telescopict_Range6_1.veiw_type,Structset_Telescopict_Range6_1.Setting_Para);
						Set_Para_to_char(Structset_Telescopict_Range6_1.Setting_Para,Structset_Telescopict_Range6_1.veiw_type,Structset_Telescopict_Range6_1.Setting_Char);
					break;
					case 7:
						Structset_PLATE_Forbiden7_3.Setting_Para=Set_Prar_Change_Input(Structset_PLATE_Forbiden7_3.veiw_type,Structset_PLATE_Forbiden7_3.Setting_Para);
						Set_Para_to_char(Structset_PLATE_Forbiden7_3.Setting_Para,Structset_PLATE_Forbiden7_3.veiw_type,Structset_PLATE_Forbiden7_3.Setting_Char);
					break;
					case 8:
						Structset_PLATE_StretchTime8_1.Setting_Para=Set_Prar_Change_Input(Structset_PLATE_StretchTime8_1.veiw_type,Structset_PLATE_StretchTime8_1.Setting_Para);
						Set_Para_to_char(Structset_PLATE_StretchTime8_1.Setting_Para,Structset_PLATE_StretchTime8_1.veiw_type,Structset_PLATE_StretchTime8_1.Setting_Char);
					break;
					case 9:
						Structset_PLATE_ShrinkTime9_1.Setting_Para=Set_Prar_Change_Input(Structset_PLATE_ShrinkTime9_1.veiw_type,Structset_PLATE_ShrinkTime9_1.Setting_Para);
						Set_Para_to_char(Structset_PLATE_ShrinkTime9_1.Setting_Para,Structset_PLATE_ShrinkTime9_1.veiw_type,Structset_PLATE_ShrinkTime9_1.Setting_Char);
					break;
					
					default:
					break;	
				}
			break;
			
			case TYPE_SETTING_GROUP_NC:

			break;
			
			case TYPE_SETTING_AUTOMATION:
				switch(Setting_Lengthways)
				{
					case 1:
//						State_Mainlogic_DWFAULTstate = AUTO_IDLE;
//						_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
						array_Follow[0] = BUS_FOLLOW_START_T;
						array_Follow[1] = Structset_Follow_craft3_6.Setting_Para;
						array_Follow[2] = Structset_Follow_Head1_1.Setting_Para;
						array_Follow[3] = Structset_Follow_Tail2_1.Setting_Para;//跟机头
						array_Follow[4] = 0;//跟机尾
						array_Follow[5] = 0;
						array_Follow[6] = 0;
						array_Follow[7] = 0;
					
					
						CANx_SendNormalData(&BUS_CAN,0xee,array_Follow,8);//
						break;
					
					case 2:
						State_Mainlogic_DWFAULTstate = NORMOL_IDLE;
						_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;
						array_Follow[0] = BUS_FOLLOW_STOP_R;
						CANx_SendNormalData(&BUS_CAN,0xff,array_Follow,8);//
						break;
					
					case 3:
						Structset_Follow_Head1_1.Setting_Para=Set_Prar_Change_Input(Structset_Follow_Head1_1.veiw_type,Structset_Follow_Head1_1.Setting_Para);
						Set_Para_to_char(Structset_Follow_Head1_1.Setting_Para,Structset_Follow_Head1_1.veiw_type,Structset_Follow_Head1_1.Setting_Char);
						break;
					
					case 4:
						Structset_Follow_Tail2_1.Setting_Para=Set_Prar_Change_Input(Structset_Follow_Tail2_1.veiw_type,Structset_Follow_Tail2_1.Setting_Para);
						Set_Para_to_char(Structset_Follow_Tail2_1.Setting_Para,Structset_Follow_Tail2_1.veiw_type,Structset_Follow_Tail2_1.Setting_Char);
						break;
					
					case 5:
						Structset_Follow_craft3_6.Setting_Para=Set_Prar_Change_Input(Structset_Follow_craft3_6.veiw_type,Structset_Follow_craft3_6.Setting_Para);
						Set_Para_to_char(Structset_Follow_craft3_6.Setting_Para,Structset_Follow_craft3_6.veiw_type,Structset_Follow_craft3_6.Setting_Char);
						break;
				}
			break;
			
			case TYPE_SETTING_FAULT:
				switch(Setting_Lengthways)
				{
					
					
				}
			break;
			
			default:
			break;	

	}

}

void Main_logic_STATEchose(void)
{	

	switch(_variant_Mainlogic_statement.PRESENTstate_value)
	{
			case URGENCY_STOP:
				_variant_Mainlogic_statement.PRESENTstate_view[0] = 0xBC;
				_variant_Mainlogic_statement.PRESENTstate_view[1] = 0xB1;
				_variant_Mainlogic_statement.PRESENTstate_view[2] = 0xCD;
				_variant_Mainlogic_statement.PRESENTstate_view[3] = 0xA3;
				_variant_Mainlogic_statement.PRESENTstate_view[4] = '\0';
				break;
			case NORMOL_IDLE:
				_variant_Mainlogic_statement.PRESENTstate_view[0] = 0xBF;
				_variant_Mainlogic_statement.PRESENTstate_view[1] = 0xD5;
				_variant_Mainlogic_statement.PRESENTstate_view[2] = 0xCF;
				_variant_Mainlogic_statement.PRESENTstate_view[3] = 0xD0;
				_variant_Mainlogic_statement.PRESENTstate_view[4] = '\0';		
				break;
			case AUTO_IDLE:
				_variant_Mainlogic_statement.PRESENTstate_view[0] = 0xB8;
				_variant_Mainlogic_statement.PRESENTstate_view[1] = 0xFA;
				_variant_Mainlogic_statement.PRESENTstate_view[2] = 0xBB;
				_variant_Mainlogic_statement.PRESENTstate_view[3] = 0xFA;
				_variant_Mainlogic_statement.PRESENTstate_view[4] = '\0';
				break;
			case SETTING:
				strcpy(_variant_Mainlogic_statement.PRESENTstate_view,"设置");
				break;
			case CONTROLED:
				strcpy(_variant_Mainlogic_statement.PRESENTstate_view,"被控");
				break;
			case FREEZE:
				strcpy(_variant_Mainlogic_statement.PRESENTstate_view,"暂停");
				break;
			case MOVE:
				strcpy(_variant_Mainlogic_statement.PRESENTstate_view,"动作");
				break;
			case URGENCY_LOCK:
				strcpy(_variant_Mainlogic_statement.PRESENTstate_view,"闭锁");
				break;	
	}
	_variant_Mainlogic_statement.PRESENTstate_view[4] = '\0';
}


void Main_Logic_Init(void)
{

	_variant_Mainlogic_statement.PRESENTstate_value = State_Mainlogic_DWFAULTstate;


}
static uint8_t Key_value_test,Key_value_test2;
static uint8_t KEY_Flag =0;
void Main_Logic_SingalMove(uint8_t Key_Value)
{
	uint16_t time=0;
//	uint8_t key_elepotential=0;
	KEY_Flag =0;
	uint8_t erro1=0;
//	while((Key_value_test == Key_Value)&&(time<100))
	while((KEY_Flag == 0)&&(time<500))
	{
		Key_value_test = CH455_Key_Read();
		time++;
//		key_elepotential = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);
		xQueueSend(CAN_Send_Queue,&_unitSingalCMD.arrayKEYSingal,0);
		if(Key_value_test != Key_Value)
		{
			erro1++;
		}
//		if(key_elepotential!=0)
//		{
//		    erro2++;
//		}
		if(erro1 >= 12 )
		{
			KEY_Flag = 1;
		}
		vTaskDelay(20);
	}
	Key_value_test = CH455_Key_Read();
	Key_value_test2 = Key_Value;
}
