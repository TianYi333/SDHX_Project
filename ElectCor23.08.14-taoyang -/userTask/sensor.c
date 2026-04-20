#include "sensor.h"
#include "main.h"
#include "main_logic.h"
#include "bsp_oled19264.h"
#include "interface.h"
#include "canreceive.h"
#include "define.h"
/**********************************************数组定义*******************************************************/



uint32_t array_adc_receive[3]={0};
uint8_t  array_Angel_receive[20] = {0};
float    array_ADC_SelectJ[10],array_ADC_SelectD[10],array_ADC_SelectP[10];
uint32_t Time_sensor_Angel;
uint8_t  array_Angel_send[3] = {0x55,0x11,0xAA};
uint8_t  Flag_Angel_receive= 0;
/**************************************************************状态变量定义**************************************************************/



uint8_t State_Infrared_Sensor = 0;     //红外状态管理标志位 创建人蒋博 创建日期2023/1/05
uint8_t State_sensor_scramcontrol = 0;     //急停标志位  创建人蒋博 创建日期2023/1/05



/***************************************************************计数变量定义************************************************************************************************************/

uint8_t   Count_sensor_i=0; //ADC采集用计数变量  创建人蒋博 创建日期2022/11/15
uint8_t   Count_sensor_a=0; //ADC采集滤波用计数变量  创建人蒋博 创建日期2022/11/15
//uint8_t   Count_sensor_scram=0; //急停计数  创建人蒋博 创建日期2023/1/05
uint8_t   Count_Infrared_Sensor = 0;     //红外计数  创建人蒋博 创建日期2023/1/05
uint8_t Sensor_Receive_Num=0;//采煤机脉冲数
/********************************************************变量定义************************************************************************************************************/
uint16_t	variant_ADC_SelectD2=0;
uint16_t	variant_Altitude_Vlaue=0;
char array_Altitude_view[5];//支架高度
float     variant_ADC_SelectP3 = 0;
float     variant_sensor_scramcontrol = 0;

/**********************************************************结构体定义*******************************************************/
 struct  _variant_sensor  _sensor_distance  ;     //位移
 struct  _variant_sensor  _sensor_pressure  ;     //压力
 struct  _variant_sensor  _sensor_Connectingrod_angle;      //连杆倾角
 struct  _variant_sensor  _sensor_canopy_angle;      //顶梁倾角
 struct  _variant_sensor  _sensor_Plate_angle;      //平衡倾角
  struct  _variant_sensor  _sensor_Foundation_angle;      //底座倾角
 
 uint16_t sin_Function(uint16_t angel);
 uint16_t angle_Function(uint16_t Plateangel,uint16_t canopyangel,uint16_t Connectingrodangel);
 
 void Function_sonser_pcReported(uint8_t type);//传感器上报函数
 
TickType_t Sensor_Time = 0; //传感器计时

/**********************************************************记忆值，记忆值不同需上报*******************************************************/
__IO uint16_t Memory_Vlaue_red = 0;

__IO uint16_t Memory_Vlaue_distance = 0;
__IO uint16_t Memory_Vlaue_pressure = 0;
__IO uint16_t Memory_Vlaue_Connectingrod_angle = 0;
__IO uint16_t Memory_Vlaue_canopy_angle = 0;
__IO uint16_t Memory_Vlaue_Plate_angle = 0;
__IO uint16_t Memory_Vlaue_Foundation_angle = 0;
uint8_t  Flag_sensor_report = 0x00; //上报标志 创建日期2023/7/25

void Sensor_Receive_rfuc(void)
{


		uint8_t i_count;
  
    uint8_t his_red = 0;


		uint16_t Angle_H;
	char test_hongwai[3];
		
		if(((Memory_Vlaue_distance - _sensor_distance.sensor_value)>=10)||((_sensor_distance.sensor_value - Memory_Vlaue_distance)>=10))
		{
			 Function_sonser_pcReported(0x01);
		}
		else if(((Memory_Vlaue_pressure - _sensor_pressure.sensor_value)>=3)||((_sensor_pressure.sensor_value - Memory_Vlaue_pressure)>=3))
		{
			 Function_sonser_pcReported(0x01);
		}
    else if(Memory_Vlaue_red != Sensor_Receive_Num)   //红外
    {
        Function_sonser_pcReported(0x01);
    }
    
		if(((Memory_Vlaue_Foundation_angle - _sensor_Foundation_angle.sensor_value  / 100)>=10)||((_sensor_Foundation_angle.sensor_value / 100 - Memory_Vlaue_Foundation_angle)>=10))
		{
			 Function_sonser_pcReported(0x02);
		}
		else if(((Memory_Vlaue_canopy_angle - _sensor_canopy_angle.sensor_value  / 100)>=10)||((_sensor_canopy_angle.sensor_value / 100 - Memory_Vlaue_canopy_angle)>=10))
		{
			   Function_sonser_pcReported(0x02);
		}
		else if(((Memory_Vlaue_Plate_angle - _sensor_Plate_angle.sensor_value / 100)>=10)||((_sensor_Plate_angle.sensor_value / 100 - Memory_Vlaue_Plate_angle)>=10))
		{
			 Function_sonser_pcReported(0x02);
		}
		else if(((Memory_Vlaue_Connectingrod_angle - _sensor_Connectingrod_angle.sensor_value / 100)>=10)||((_sensor_Connectingrod_angle.sensor_value / 100 - Memory_Vlaue_Connectingrod_angle)>=10))
		{
			  Function_sonser_pcReported(0x02);
		}
		
		
		

			for(Count_sensor_i=0;Count_sensor_i<3;Count_sensor_i++)
			{
					HAL_ADC_Start(&hadc1);
					HAL_ADC_PollForConversion(&hadc1,0xffff);
					array_adc_receive[Count_sensor_i]= hadc1.Instance->DR;
					vTaskDelay(2);
                    
                    
			}
			HAL_ADC_Stop(&hadc1);
			
//			array_adc_receive[0]  =  array_adc_receive[0]*2/1024;
//			array_adc_receive[1]  =  array_adc_receive[1]*2/1024;
//			array_adc_receive[2]  =  array_adc_receive[2]*2/1024;
			
			if(Count_sensor_a<10)
			{
					array_ADC_SelectJ[Count_sensor_a] = array_adc_receive[0];
					array_ADC_SelectD[Count_sensor_a] = array_adc_receive[1];
					array_ADC_SelectP[Count_sensor_a] = array_adc_receive[2];
					Count_sensor_a++;
			}	
			
			if(Count_sensor_a>=10)
			{
					Count_sensor_a=0;
					variant_sensor_scramcontrol  = (uint16_t)((array_ADC_SelectJ[0]+array_ADC_SelectJ[1]+array_ADC_SelectJ[2]+array_ADC_SelectJ[3]+array_ADC_SelectJ[4]+array_ADC_SelectJ[5]
                    +array_ADC_SelectJ[6]+array_ADC_SelectJ[7]+array_ADC_SelectJ[8]+array_ADC_SelectJ[9])/10);
                    //急停电压检测
					variant_ADC_SelectD2 = (uint16_t)((array_ADC_SelectD[0]+array_ADC_SelectD[1]+array_ADC_SelectD[2]+array_ADC_SelectD[3]+array_ADC_SelectD[4]+array_ADC_SelectD[5]
                    +array_ADC_SelectD[6]+array_ADC_SelectD[7]+array_ADC_SelectD[8]+array_ADC_SelectD[9])/10);
                    //行程电压检测
					variant_ADC_SelectP3 = (uint16_t)((array_ADC_SelectP[0]+array_ADC_SelectP[1]+array_ADC_SelectP[2]+array_ADC_SelectP[3]+array_ADC_SelectP[4]+array_ADC_SelectP[5]
                    +array_ADC_SelectP[6]+array_ADC_SelectP[7]+array_ADC_SelectP[8]+array_ADC_SelectP[9])/10);
                    //压力电压检测 注意没有除十，显示时小数点直接位移一位
            
				//换算传感器示数
					
					if(((variant_ADC_SelectD2*330)/2048)>=10)
					{
				
						_sensor_distance.sensor_value = (uint16_t)((float)((((variant_ADC_SelectD2*330)/2048 - SENSOR_DISTANCE_VOLTAGE_L)* SENSOR_DISTANCE_RABGE)/SENSOR_DISTANCE_VOLTAGE_V )); //行程
						
							
						_sensor_distance.sensor_view[0] = (_sensor_distance.sensor_value/1000)+48;
						_sensor_distance.sensor_view[1] = ((_sensor_distance.sensor_value%1000)/100)+48;
						_sensor_distance.sensor_view[2] = ((_sensor_distance.sensor_value%100)/10)+48;
						_sensor_distance.sensor_view[3] = (_sensor_distance.sensor_value%10)+48;
						_sensor_distance.sensor_view[4] = 'm';
						_sensor_distance.sensor_view[5] = 'm';
						_sensor_distance.sensor_view[6] = ' ';
						_sensor_distance.sensor_view[7] = '\0';
						
					}
					
					else if(((variant_ADC_SelectD2*330)/2048)<10)
					{
					
						_sensor_distance.sensor_view[0] = 'v';
						_sensor_distance.sensor_view[1] = 'v';
						_sensor_distance.sensor_view[2] = 'v';
						_sensor_distance.sensor_view[3] = 'v';
						_sensor_distance.sensor_view[4] = 'm';
						_sensor_distance.sensor_view[5] = 'm';
						_sensor_distance.sensor_view[6] = ' ';
						_sensor_distance.sensor_view[7] = '\0';

					}
					
					if(((variant_ADC_SelectP3*330)/2048)>=49)
					{
						_sensor_pressure.sensor_value = (uint16_t)((float)(((variant_ADC_SelectP3*330)/2048 - SENSOR_PRESSURE_VOLTAGE_L)*SENSOR_PRESSURE_RABGE)/ SENSOR_PRESSURE_VOLTAGE_V); //压力
				
						
					
						
						_sensor_pressure.sensor_view[0] = (_sensor_pressure.sensor_value/100)+48;
						_sensor_pressure.sensor_view[1] = ((_sensor_pressure.sensor_value%100)/10)+48;
						_sensor_pressure.sensor_view[2] = 0x2E;
						_sensor_pressure.sensor_view[3] = (_sensor_pressure.sensor_value%10)+48;
						_sensor_pressure.sensor_view[4] = 'M';
						_sensor_pressure.sensor_view[5] = 'P';
						_sensor_pressure.sensor_view[6] = 'a';
						_sensor_pressure.sensor_view[7] = '\0';
					}

					else if(((variant_ADC_SelectP3*330)/2048)<=10)
					{
					
						_sensor_pressure.sensor_view[0] = 'v';
						_sensor_pressure.sensor_view[1] = 'v';
						_sensor_pressure.sensor_view[2] = 'v';
						_sensor_pressure.sensor_view[3] = 'v';
						_sensor_pressure.sensor_view[4] = 'M';
						_sensor_pressure.sensor_view[5] = 'P';
						_sensor_pressure.sensor_view[6] = 'a';
						_sensor_pressure.sensor_view[7] = '\0';

					}
					
			}	
			


				
				if(500<=(TIME - Sensor_Time))
				{
          //记录历史红外数据
          his_red = Sensor_Receive_Num;
          
          if(HAL_UART_Receive(&huart3,&Sensor_Receive_Num,1,0) != HAL_OK)
          {
            //接收失败时赋回之前的值
            Sensor_Receive_Num = his_red;
          }
          Sensor_Time = TIME;

				}
        else if(TIME < Sensor_Time)
        {
          //溢出后重置Sensor_Time
          Sensor_Time = TIME;
        }
				
				
				

				
					switch(array_Angel_send[1])
					{
						case 0x11:
							 array_Angel_send[1] = 0x12;
							break;
						case 0x12:
							 array_Angel_send[1] = 0x13;
							break;
						case 0x13:
							 array_Angel_send[1] = 0x14;
							break;
						case 0x14:
							 array_Angel_send[1] = 0x11;
							break;
						default:
							break;
					
					
					}
//				HAL_UART_Transmit(&huart7,array_Angel_send,3,100);
					HAL_UART_Receive_DMA(&huart7,array_Angel_receive,20);
					usart7_niming_report(3);
//					Sensor_angeluart7_send_data(array_Angel_send,3);
					vTaskDelay(300);
					
				HAL_UART_DMAStop(&huart7);
				for(i_count=0;i_count<12;i_count++)
				{
					if((array_Angel_receive[i_count] == 0x55)&&(array_Angel_receive[i_count+8] == 0xAA)&&(array_Angel_receive[i_count+2] != 0xFF))
					{
						
							if(array_Angel_receive[i_count+1] == 0x02)
							{
								Angle_H = array_Angel_receive[i_count+3];
								_sensor_Connectingrod_angle .sensor_value = (Angle_H<<8) +	array_Angel_receive[i_count+4];
								_sensor_Connectingrod_angle.sensor_view[0] = array_Angel_receive[i_count+2];
							}
							else if(array_Angel_receive[i_count+1] == 0x01)
							{
								Angle_H = array_Angel_receive[i_count+3];
								_sensor_canopy_angle .sensor_value = (Angle_H<<8) +	array_Angel_receive[i_count+4];
								_sensor_canopy_angle.sensor_view[0] = array_Angel_receive[i_count+2];						
							}
							else if(array_Angel_receive[i_count+1] == 0x03)
							{
								Angle_H = array_Angel_receive[i_count+3];
								_sensor_Plate_angle .sensor_value = (Angle_H<<8) +	array_Angel_receive[i_count+4];
								_sensor_Plate_angle.sensor_view[0] = array_Angel_receive[i_count+2];						
							}
							else if(array_Angel_receive[i_count+1] == 0x04)
							{
								Angle_H = array_Angel_receive[i_count+3];
								_sensor_Foundation_angle.sensor_value = (Angle_H<<8) +	array_Angel_receive[i_count+4];
								_sensor_Foundation_angle.sensor_view[0] = array_Angel_receive[i_count+2];						
							}

					
						memset(array_Angel_receive,0,20);
/***********************************************************************************************判断倾角的正负********************************************************************************************************/						

//						HAL_UART_DMAStop(&huart7);
						
						
						
						_sensor_Connectingrod_angle.sensor_view[1] = ((_sensor_Connectingrod_angle.sensor_value%100000)/10000)+48;
						_sensor_Connectingrod_angle.sensor_view[2] = ((_sensor_Connectingrod_angle.sensor_value%10000)/1000)+48;
						_sensor_Connectingrod_angle.sensor_view[3] = ((_sensor_Connectingrod_angle.sensor_value%1000)/100)+48;
						_sensor_Connectingrod_angle.sensor_view[4] = 0x2E;
						_sensor_Connectingrod_angle.sensor_view[5] = ((_sensor_Connectingrod_angle.sensor_value%100)/10)+48;
						_sensor_Connectingrod_angle.sensor_view[6] = ' ';
						_sensor_Connectingrod_angle.sensor_view[7] = '\0';
						
						_sensor_canopy_angle.sensor_view[1] = ((_sensor_canopy_angle.sensor_value%100000)/10000)+48;
						_sensor_canopy_angle.sensor_view[2] = ((_sensor_canopy_angle.sensor_value%10000)/1000)+48;
						_sensor_canopy_angle.sensor_view[3] = ((_sensor_canopy_angle.sensor_value%1000)/100)+48;
						_sensor_canopy_angle.sensor_view[4] = 0x2E;
						_sensor_canopy_angle.sensor_view[5] = ((_sensor_canopy_angle.sensor_value%100)/10)+48;
						_sensor_canopy_angle.sensor_view[6] =' ';
						_sensor_canopy_angle.sensor_view[7] = '\0';
						
						_sensor_Plate_angle.sensor_view[1] = ((_sensor_Plate_angle.sensor_value%100000)/10000)+48;
						_sensor_Plate_angle.sensor_view[2] = ((_sensor_Plate_angle.sensor_value%10000)/1000)+48;
						_sensor_Plate_angle.sensor_view[3] = ((_sensor_Plate_angle.sensor_value%1000)/100)+48;
						_sensor_Plate_angle.sensor_view[4] = 0x2E;
						_sensor_Plate_angle.sensor_view[5] = ((_sensor_Plate_angle.sensor_value%100)/10)+48;
						_sensor_Plate_angle.sensor_view[6] = ' ';
						_sensor_Plate_angle.sensor_view[7] ='\0';

						_sensor_Foundation_angle.sensor_view[1] = ((_sensor_Foundation_angle.sensor_value%100000)/10000)+48;
						_sensor_Foundation_angle.sensor_view[2] = ((_sensor_Foundation_angle.sensor_value%10000)/1000)+48;
						_sensor_Foundation_angle.sensor_view[3] = ((_sensor_Foundation_angle.sensor_value%1000)/100)+48;
						_sensor_Foundation_angle.sensor_view[4] = 0x2E;
						_sensor_Foundation_angle.sensor_view[5] = ((_sensor_Foundation_angle.sensor_value%100)/10)+48;
						_sensor_Foundation_angle.sensor_view[6] = ' ';
						_sensor_Foundation_angle.sensor_view[7] ='\0';

				
						memset(array_Angel_receive,0,20);	
					}
						variant_Altitude_Vlaue = angle_Function(_sensor_Plate_angle.sensor_value,_sensor_canopy_angle.sensor_value,_sensor_Connectingrod_angle.sensor_value);

					    array_Altitude_view[0] = (variant_Altitude_Vlaue/1000)+48;
						array_Altitude_view[1] = ((variant_Altitude_Vlaue%1000)/100)+48;
						array_Altitude_view[2] = ((variant_Altitude_Vlaue%100)/10)+48;
						array_Altitude_view[3] = (variant_Altitude_Vlaue%10)+48;
						array_Altitude_view[4] = '\0';				
			}
				

}


void usart7_send_char(uint8_t c)
{
	while((UART7->SR&0X40)==0);//等待上一次发送完毕   
	UART7->DR=c;   	
}
void usart7_niming_report(uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)usart7_send_char(array_Angel_send[i]);
}



void Sensor_angeluart7_send_data(uint8_t *tdata,uint16_t tnum)
{
        //等待发送状态OK
        while(HAL_DMA_GetState(&hdma_uart7_tx) == HAL_DMA_STATE_BUSY)
			vTaskDelay(1);
        //发送数据
        HAL_UART_Transmit_DMA(&huart7,tdata,tnum);
}
uint16_t angle_Function(uint16_t Plateangel,uint16_t canopyangel,uint16_t Connectingrodangel)
{
	uint16_t Altitude = 0;
	uint16_t Plateangel_value = 0;
	uint16_t Connectingrodangel_value = 0;
	Plateangel_value = sin_Function(Plateangel);
	Connectingrodangel_value = sin_Function(Connectingrodangel);
	Altitude = 720+(Plateangel_value*75/100)+(Connectingrodangel_value+Connectingrodangel_value/2);
	return Altitude;
}
uint16_t sin_Function(uint16_t angel)
{
	 uint16_t sinvalue;
	/*进行sin计算*/
	 uint16_t angalvalue;//真实角度
     if(angel>9000)//如果大于90度（角度值小数点两位，相当于*100），则取补角
	 {
		angalvalue = 18000-angel;
	 }
	 else if(angel<9000)//如果小于90度，则取值
	 {
		angalvalue = angel;
	 }
	angalvalue = angalvalue/100;
	if(angalvalue<=30)		
	{
		switch(angalvalue)
		{
			case 1:
				sinvalue = 17;
				break;
			case 2:
				sinvalue = 35;
				break;
			case 3:
				sinvalue = 52;
				break;
			case 4:
				sinvalue = 70;
				break;
			case 5:
				sinvalue = 87;
				break;
			case 6:
				sinvalue = 104;
				break;
			case 7:
				sinvalue = 122;
				break;
			case 8:
				sinvalue = 139;
				break;
			case 9:
				sinvalue = 156;
				break;
			case 10:
				sinvalue = 174;
				break;
			case 11:
				sinvalue = 191;
				break;
			case 12:
				sinvalue = 208;
				break;
			case 13:
				sinvalue = 225;
				break;
			case 14:
				sinvalue = 242;
				break;
			case 15:
				sinvalue = 259;
				break;
			case 16:
				sinvalue = 275;
				break;
			case 17:
				sinvalue = 292;
				break;
			case 18:
				sinvalue = 309;
				break;
			case 19:
				sinvalue = 326;
				break;
			case 20:
				sinvalue = 342;
				break;
			case 21:
				sinvalue = 358;
				break;
			
			case 22:
				sinvalue = 375;
				break;
			case 23:
				sinvalue = 391;
				break;
			case 24:
				sinvalue = 407;
				break;
			case 25:
				sinvalue = 423;
				break;
			case 26:
				sinvalue = 438;
				break;
			case 27:
				sinvalue = 454;
				break;
			case 28:
				sinvalue = 469;
				break;
			case 29:
				sinvalue = 485;
				break;
			case 30:
				sinvalue = 500;
				break;
			
			default:
				break;
				
		}
	}
	else if(angalvalue<=60)		
	{
		switch(angalvalue)
		{
			case 31:
				sinvalue = 515;
				break;
			case 32:
				sinvalue = 530;
				break;
			case 33:
				sinvalue = 545;
				break;
			case 34:
				sinvalue = 559;
				break;
			case 35:
				sinvalue = 574;
				break;
			case 36:
				sinvalue = 588;
				break;
			case 37:
				sinvalue = 602;
				break;
			case 38:
				sinvalue = 616;
				break;
			case 39:
				sinvalue = 629;
				break;
			case 40:
				sinvalue = 643;
				break;
			case 41:
				sinvalue = 656;
				break;
			case 42:
				sinvalue = 669;
				break;
			case 43:
				sinvalue = 682;
				break;
			case 44:
				sinvalue = 695;
				break;
			case 45:
				sinvalue = 707;
				break;
			case 46:
				sinvalue = 719;
				break;
			case 47:
				sinvalue = 731;
				break;
			case 48:
				sinvalue = 743;
				break;
			case 49:
				sinvalue = 755;
				break;
			case 50:
				sinvalue = 766;
				break;
			case 51:
				sinvalue = 777;
				break;
			
			case 52:
				sinvalue = 788;
				break;
			case 53:
				sinvalue = 798;
				break;
			case 54:
				sinvalue = 809;
				break;
			case 55:
				sinvalue = 819;
				break;
			case 56:
				sinvalue = 829;
				break;
			case 57:
				sinvalue = 839;
				break;
			case 58:
				sinvalue = 848;
				break;
			case 59:
				sinvalue = 857;
				break;
			case 60:
				sinvalue = 866;
				break;
			
			default:
				break;
				
		}
	}	
	else if(angalvalue>60)		
	{
		switch(angalvalue)
		{
			case 61:
				sinvalue = 875;
				break;
			case 62:
				sinvalue = 883;
				break;
			case 63:
				sinvalue = 891;
				break;
			case 64:
				sinvalue = 899;
				break;
			case 65:
				sinvalue = 906;
				break;
			case 66:
				sinvalue = 914;
				break;
			case 67:
				sinvalue = 921;
				break;
			case 68:
				sinvalue = 927;
				break;
			case 69:
				sinvalue = 934;
				break;
			case 70:
				sinvalue = 940;
				break;
			case 71:
				sinvalue = 946;
				break;
			case 72:
				sinvalue = 951;
				break;
			case 73:
				sinvalue = 956;
				break;
			case 74:
				sinvalue = 961;
				break;
			case 75:
				sinvalue = 966;
				break;
			case 76:
				sinvalue = 970;
				break;
			case 77:
				sinvalue = 974;
				break;
			case 78:
				sinvalue = 978;
				break;
			case 79:
				sinvalue = 982;
				break;
			case 80:
				sinvalue = 985;
				break;
			case 81:
				sinvalue = 988;
				break;
			
			case 82:
				sinvalue = 990;
				break;
			case 83:
				sinvalue = 993;
				break;
			case 84:
				sinvalue = 995;
				break;
			case 85:
				sinvalue = 996;
				break;
			case 86:
				sinvalue = 998;
				break;
			case 87:
				sinvalue = 999;
				break;
			case 88:
				sinvalue = 999;
				break;
			case 89:
				sinvalue = 999;
				break;
			case 90:
				sinvalue = 1000;
				break;
			
			default:
				break;
				
		}
	}
	return sinvalue;

}

void Function_sonser_pcReported(uint8_t type)//
{
	uint8_t Array_send[8] = {0};
	uint8_t Angal = 0x00;
	if(type == 0x01)
	{
		Array_send[0] = BUS_SEND_BUSCHEAK1;
		Array_send[1] = Structset_Service_DeviceNum1_1.Setting_Para;
		Array_send[2] = Sensor_Receive_Num;
		Array_send[3] = (_sensor_pressure.sensor_value>>8)&0xff;
		Array_send[4] = _sensor_pressure.sensor_value&0xff;
		Array_send[5] = (_sensor_distance.sensor_value>>8)&0xff;
		Array_send[6] = _sensor_distance.sensor_value&0xff;
		Array_send[7] = 0;
    
    Memory_Vlaue_distance = _sensor_distance.sensor_value;
    Memory_Vlaue_pressure = _sensor_pressure.sensor_value;
    Memory_Vlaue_red = Sensor_Receive_Num;
	}
	else if(type == 0x02)
	{
		if(_sensor_canopy_angle.sensor_view[0] == 0x2B)
		{
					Angal = Angal| 0x01;
		}
		if(_sensor_Connectingrod_angle.sensor_view[0] == 0x2B)
		{
					Angal = Angal| 0x02;
		}
		if(_sensor_Plate_angle.sensor_view[0] == 0x2B)
		{
					Angal = Angal| 0x04;
		}
		if(_sensor_Foundation_angle.sensor_view[0] == 0x2B)
		{
					Angal = Angal| 0x08;
		}
		Array_send[0] = BUS_SEND_BUSCHEAK2;
		Array_send[1] = Structset_Service_DeviceNum1_1.Setting_Para;
		Array_send[2] = Angal;
		Array_send[3] = _sensor_canopy_angle.sensor_value / 100;
		Array_send[4] = _sensor_Connectingrod_angle.sensor_value / 100;
		Array_send[5] = _sensor_Plate_angle.sensor_value / 100;
		Array_send[6] = _sensor_Foundation_angle.sensor_value / 100;
		Array_send[7] = 0;
    
    Memory_Vlaue_Connectingrod_angle = _sensor_Connectingrod_angle.sensor_value / 100;
    Memory_Vlaue_canopy_angle = _sensor_canopy_angle.sensor_value / 100;
    Memory_Vlaue_Plate_angle = _sensor_Plate_angle.sensor_value / 100;
    Memory_Vlaue_Foundation_angle = _sensor_Foundation_angle.sensor_value / 100;
	}

	CANx_SendNormalData(&BUS_CAN, PCID, Array_send, 8);	







}
