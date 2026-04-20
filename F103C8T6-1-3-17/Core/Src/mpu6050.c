#include "mpu6050.h"

#define MPU_ERROR 		I2C_ERROR
#define MPU_INFO 		I2C_INFO
/**
  * @brief   写数据到MPU6050寄存器
  * @param   reg_add:寄存器地址
	* @param	 reg_data:要写入的数据
  * @retval  
  */
void MPU6050_WriteReg(uint8_t reg_add,uint8_t reg_dat)
{
	Sensors_I2C_WriteRegister(MPU6050_ADDRESS,reg_add,1,&reg_dat);
}

/**
  * @brief   从MPU6050寄存器读取数据
  * @param   reg_add:寄存器地址
	* @param	 Read：存储数据的缓冲区
	* @param	 num：要读取的数据量
  * @retval  
  */
void MPU6050_ReadData(uint8_t reg_add,uint8_t *Read,uint8_t num)
{
	Sensors_I2C_ReadRegister(MPU6050_ADDRESS,reg_add,num,Read);
}


/**
  * @brief   初始化MPU6050芯片
  * @param   
  * @retval  
  */
void MPU6050_Init(void)
{
	HAL_Delay(100);	                                //在初始化之前要延时一段时间，若没有延时，则断电后再上电数据可能会出错
	MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x00);	//解除休眠状态
	MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV , 0x07);	//陀螺仪采样率
	MPU6050_WriteReg(MPU6050_RA_CONFIG , 0x06);	
	MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG , 0x00);//配置加速度传感器工作在2G模式
	MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18); //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
	HAL_Delay(200);
}

/**
  * @brief   读取MPU6050的ID
  * @param   
  * @retval  正常返回1，异常返回0
  */
uint8_t MPU6050ReadID(void)
{
	unsigned char Re = 0;
    MPU6050_ReadData(MPU6050_RA_WHO_AM_I,&Re,1);  //读器件地址
	if(Re != 0x68)
	{
		MPU_ERROR("MPU6050 dectected error!\r\n检测不到MPU6050模块，请检查模块与开发板的接线");
		return 0;
	}
	else
	{
		MPU_INFO("MPU6050 ID = %d\r\n",Re);
		return 1;
	}
		
}

/**
  * @brief   读取MPU6050的加速度数据
  * @param   
  * @retval  
  */
void MPU6050ReadAcc(short *ax,short *ay,short *az)
{
    uint8_t buf[6];
    MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);
    *ax = (buf[0] << 8) | buf[1];									//陀螺仪x轴的原始读数(带符号)
    *ay = (buf[2] << 8) | buf[3];									//陀螺仪y轴的原始读数(带符号)
    *az = (buf[4] << 8) | buf[5];									//陀螺仪z轴的原始读数(带符号)
}

/**
  * @brief   读取MPU6050的角加速度数据
  * @param   
  * @retval  
  */
void MPU6050ReadGyro(short *gx,short *gy,short *gz)
{
    uint8_t buf[6];
    MPU6050_ReadData(MPU6050_GYRO_OUT,buf,6);
    *gx = (buf[0] << 8) | buf[1];									//陀螺仪x轴加速度的原始读数(带符号)
    *gy = (buf[2] << 8) | buf[3];									//陀螺仪y轴加速度的原始读数(带符号)
    *gz = (buf[4] << 8) | buf[5];									//陀螺仪z轴加速度的原始读数(带符号)
}

/**
  * @brief   读取MPU6050的原始温度数据
  * @param   
  * @retval  
  */
void MPU6050ReadTemp(short *tempData)
{
	uint8_t buf[2];
    MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);//读取温度值
    *tempData = (buf[0] << 8) | buf[1];
}

/**
  * @brief   读取MPU6050的温度数据，转化成摄氏度
  * @param   
  * @retval  
  */
void MPU6050_ReturnTemp(float *Temperature)
{
	short temp3;
	uint8_t buf[2];
	
	MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);  //读取温度值
    temp3= (buf[0] << 8) | buf[1];	
	*Temperature=((double) temp3/340.0)+36.53;      //转换为摄氏度温度值

}
