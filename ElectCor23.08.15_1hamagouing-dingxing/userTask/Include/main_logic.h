#ifndef __MAIN_LOGIC_H__
#define __MAIN_LOGIC_H__
#include "stm32f4xx_hal.h"
#include "parameter_operation.h"

typedef uint32_t  u32;
typedef uint16_t u16;
typedef uint8_t  u8;

/*******************_variant_Mainlogic_statement.PRESENTstate用，为当前状态******************/
#define   URGENCY_STOP    0x00
#define   NORMOL_IDLE     0x01
#define   AUTO_IDLE       0x02
#define   SETTING       0x03
#define   CONTROLED       0x04
#define   GROUP_CONTROLED       0x07
#define   FREEZE       0x05
#define   MOVE       0x06
#define   URGENCY_LOCK       0x0a

/*******************_variant_Mainlogic_singleCMD.movetype用，为单动动作类型******************/
#define   SINGAL_CANCEI    0x00
#define   SINGAL_PITCH_ON     0x01
#define   SINGAL_MOVE_START       0x02
/*******************_variant_Mainlogic_singleCMD.direction用，为单动动作类型******************/

#define   DIRECTION_L_LEFT    0x11
#define   DIRECTION_R_LEFT    0x21
#define   DIRECTION_L_RIGHT   0x12
#define   DIRECTION_R_RIGHT   0x22
#define   DIRECTION_LEFT    0x01
#define   DIRECTION_RIGHT   0x02
#define   DIRECTION_ROMATE   0x03
#define   DIRECTION_COMPUTER   0x04





/*******************_variant_Mainlogic_CMD.singlemoveCMD******************/
#define   MOVE_NONE                                0          //无
#define   MOVE_COLUMN_UP                                 0x01            //升立柱
#define   MOVE_COLUMN_DOWN                               0x02         //降立柱
#define   MOVE_HYDRAULICFRAME_ADVANCING                   0x03          //移架
#define   MOVE_CHUTE_PUSH                                 0x04         //推溜
#define   MOVE_BOTTOM_UP      	                         0x05        //抬底
#define   MOVE_FUEL_SPRAY                                 0x06            //喷雾
#define   MOVE_UPLIFT_BOTTOM_COLUMN_DOWN                 0x07          //抬底降柱
#define   MOVE_UPLIFT_BOTTOM_HYDRAULIC_ADVANCING        0x08            //抬底移架
#define   MOVE_BALANCE_BEAM_STRETCH                         0x09            //伸平衡梁
#define   MOVE_BALANCE_BEAM_WITHDRAW                         0x0a           	//收平衡梁
#define   MOVE_SIDE_PLATE_STRETCH                          0x0b           //伸侧推
#define   MOVE_SIDE_PLATE_WITHDRAW                         0x0c          //收侧推
#define   MOVE_TELESCOPIC_GIRDER_STRETCH                  0x0d         //伸伸缩梁
#define   MOVE_TELESCOPIC_GIRDER_WITHDRAW                  0x0e            //收伸缩梁
#define   MOVE_PLATE_STRETCH                            0x0f            //伸护帮板
#define   MOVE_PLATE_WITHDRAW                           0x10             //收护帮板
#define   MOVE_HYDRAULIC_BOTTOM_STRETCH                0x11         //伸底调
#define   MOVE_HYDRAULIC_BOTTOM_WITHDRAW               0x12           //收底调
#define   MOVE_DROP_AD_UP                 0x13             //降移升
#define   MOVE_STANDBY_APPLICATION1                0x14             //备用动作1
#define   MOVE_STANDBY_APPLICATION2                 0x15             //备用动作2
#define   MOVE_BACK_WASH2                0x16             //反冲洗1
#define   MOVE_BACK_WASH1                0x17             //反冲洗2


#define   MOVE_GROUP_ADVANCING                           0x20          //移架
#define   MOVE_GROUP_CHUTE_PUSH                          0x21          //推溜
#define   MOVE_GROUPTELESCOPICPLATE_GIRDER_WITHDRAW      0x24          //收伸缩梁护帮板
#define   MOVE_GROUP_CHUTE_PULL                          0x22          //拉溜
#define   MOVE_GROUP_TELESCOPIC_GIRDER_STRETCH           0x23          //伸伸缩梁
#define   MOVE_GROUP_PLATE_STRETCH                       0x25          //伸护帮板
#define   MOVE_GROUP_PLATE_WITHDRAW                       0x26          //收护帮板



#define   MOVE_NONE_OUT                                0          //无
#define   MOVE_COLUMN_UP_OUT                                 (1<<23)            //升立柱
#define   MOVE_COLUMN_DOWN_OUT                              (1<<22)         //降立柱
#define   MOVE_HYDRAULICFRAME_ADVANCING_OUT                  (1<<21)          //移架
#define   MOVE_CHUTE_PUSH_OUT                                (1<<20)         //推溜
#define   MOVE_BOTTOM_UP_OUT      	                        (1<<4)         //抬底
#define   MOVE_FUEL_SPRAY_OUT                                (1<<5)            //喷雾
#define   MOVE_BALANCE_BEAM_STRETCH_OUT                        (1<<19)            //伸平衡梁
#define   MOVE_BALANCE_BEAM_WITHDRAW_OUT                         (1<<18)            	//收平衡梁
#define   MOVE_SIDE_PLATE_STRETCH_OUT                         (1<<9)           //伸侧推
#define   MOVE_SIDE_PLATE_WITHDRAW_OUT                        (1<<8)           //收侧推
#define   MOVE_TELESCOPIC_GIRDER_STRETCH_OUT                 (1<<13)          //伸伸缩梁
#define   MOVE_TELESCOPIC_GIRDER_WITHDRAW_OUT                 (1<<12)            //收伸缩梁
#define   MOVE_PLATE_STRETCH_OUT                           (1<<17)            //伸护帮板
#define   MOVE_PLATE_WITHDRAW_OUT                          (1<<16)             //收护帮板
#define   MOVE_HYDRAULIC_BOTTOM_STRETCH_OUT               (1<<3)           //伸底调
#define   MOVE_HYDRAULIC_BOTTOM_WITHDRAW_OUT              (1<<2)           //收底调
#define   MOVE_STANDBY_APPLICATION1_OUT                  (1<<6)             //备用动作1
#define   MOVE_STANDBY_APPLICATION2_OUT                   (1<<7)           //备用动作2

///***************************variant_Mainlogic_Setting_subdirectory;//设置界面横向子菜单  创建人蒋博 创建日期2023/02/9*********************************/
//#define   SETING_COMMUNICATION_RAGE                         0x00          //通信范围
//#define   SETING_GLOBAL_SET                                 0x01          //整体设置
//#define   SETING_SERVICE                                    0x02          //服务
//#define   SETING_DEFAULT_PARAMETERS                         0x03          //缺省参数
//#define   SETING_GROUP_ADVANCING                                0x04          //成组移架
//#define   SETING_GROUP_CHUTE_PUSH     	                        0x05          	//成组推溜
//#define   SETING_GROUP_BALANCE                 0x06          //成组平衡
//#define   SETING_GROUP_TELESCOPIC         0x07           //成组伸缩护帮
//#define   SETING_GROUP_NC                         0x08           //其他
//#define   SETING_AUTOMATION                        0x09          	//跟机自动化
//#define   SETING_FAULT                          0x0a          //故障显示


/**********************************************************结构体定义*************************************************************************************/




/**********************************************************结构体定义*************************************************************************************/
typedef struct _variant_statement{
			
			u8 PRESENTstate_value;		//状态值
			char PRESENTstate_view[5];

}_variant_statement;



typedef struct _variant_mainlogicCMD1{
	u8 type;		
	u8 direction;		//单动方向
	u8 movetype;		//单动动作类型
	u8 num;		//  控制到第几架单动
	u8 back_Flag;  //
	u8 moveCMD;		//单动动作命令码
}_variant_mainlogicCMD1;



typedef struct _variant_mainlogicCMD2{
	u8 type0;		
	u8 direction1;		//成组方向
	u8 alarmnum2;		//成组报警数
	u8 movenum3;		        //  成组动作范围
	u8 moveCMD4;		    //单动动作命令码
	u8 limit5;		    //单动动作命令码
	u8 gap6;		    //隔离架
	u8 controlnum7;		    //成组动作发起者id
}_variant_mainlogicCMD2;



union _unit_mainlogic_Singal{

		uint8_t arrayKEYSingal[6];
		struct _variant_mainlogicCMD1 Mainlogic_singalCMD;

};


union _unit_mainlogic_Group{

		uint8_t arrayKEYGroup[8];
		struct _variant_mainlogicCMD2 Mainlogic_groupCMD;

};


/*******************************************对外声明************************************************************************************************************/
void Main_logic(void);
void Main_logic_STATEchose(void);
	
void Main_Logic_Init(void);
void Set_Prar_Change(uint8_t Setting_Crosswise,uint8_t Setting_Lengthways);

 extern   _variant_statement _variant_Mainlogic_statement;  




extern uint8_t variant_Mainlogic_KeyValue;
extern u8 State_Mainlogic_PRESENT;
extern uint8_t State_Mainlogic_DWFAULTstate;
extern uint8_t const_Mainlogic_InterfaceFlag;//屏显更新指令  创建人蒋博 创建日期2023/01/17
void Group_Pause_Count(void);

extern uint8_t Flag_Mainlogic_Groupcontorl ;//该设备有一个需负责的成组控制
extern uint8_t variant_Mainlogic_GroupMovenum;//该设备有一个需负责的成组控制
extern uint8_t Flag_Mainlogic_croupmovecancel ;//接收数组
void Main_Logic_SingalMove(uint8_t Key_Value);
#endif

   



