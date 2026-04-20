#include "bsp_oled19264.h"

extern uint8_t gb2312_code[1072];
extern uint8_t space_code[16];
static void OLED19264_Delay(void)
{
	uint8_t i;
	for (i = 0; i < 35; i++)
		;
}

static void OLED_Delay(uint16_t time)
{
	for (int i = 0; i < time; i++)
		;
}
/**
 *@brief 产生起始信号
 *@param void
 *@retval void
 */
void I2C_Start(void)
{
	OLED_I2C_SDA_H;
	OLED_I2C_SCL_H;
	OLED19264_Delay();

	OLED_I2C_SDA_L;
	OLED19264_Delay();
	OLED_I2C_SCL_L;
	OLED19264_Delay();
}

/**
 *@brief 产生停止信号
 *@param void
 *@retval void
 */
void I2C_Stop(void)
{

	OLED_I2C_SDA_L;
	OLED_I2C_SCL_L;
	OLED19264_Delay();

	OLED_I2C_SCL_H;
	OLED19264_Delay();
	OLED_I2C_SDA_H;
	OLED19264_Delay();
}

/**
 *@brief 发送一个字节
 *@param 要发送的数据
 *@retval void
 */
void I2C_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	for (i = 0; i < 8; i++)
	{
		OLED_I2C_SCL_L;
		if (_ucByte & 0x80)
		{
			OLED_I2C_SDA_H;
		}
		else
		{
			OLED_I2C_SDA_L;
		}
		OLED19264_Delay();
		OLED_I2C_SCL_H;
		OLED19264_Delay();
		_ucByte <<= 1;
	}
	OLED_I2C_SCL_L;
	OLED19264_Delay();
	//	OLED_I2C_SCL_H;
}

/**
 *@brief 等待从机应答
 *@param void
 *@retval 如果从机应答，返回1
 */
uint8_t I2C_WaitAck(void)
{
	uint8_t re = 0;

	OLED_I2C_SDA_L;

	OLED19264_Delay();

	OLED_I2C_SCL_H;

	OLED19264_Delay();

	OLED_I2C_SCL_L;

	OLED19264_Delay();

	OLED_I2C_SDA_H;

	OLED19264_Delay();

	return re;
}

/**
*@brief 发送命令
 *@param void
 *@retval 命令
 */
void OLED19264_WRITE_COMMAND(uint8_t com)
{
	I2C_Start();
	I2C_SendByte(0x7A);
	I2C_WaitAck();
	I2C_SendByte(0x00);
	I2C_WaitAck();
	I2C_SendByte(com);
	I2C_WaitAck();
	I2C_Stop();
}

/**
 *@brief 发送数据
 *@param void
 *@retval void
 */
void OLED19264_WRITE_DATA(uint8_t data)
{
	I2C_Start();
	I2C_SendByte(0x7A);
	I2C_WaitAck();
	I2C_SendByte(0x40);
	I2C_WaitAck();
	I2C_SendByte(data);
	I2C_WaitAck();
	I2C_Stop();
}

/**
 *@brief 初始化
 *@param void
 *@retval void
 */
void OLED19264_INIT(void)
{
	HAL_GPIO_WritePin(OLED_I2C_PORT, OLED_I2C_CS, GPIO_PIN_RESET); //片选

	/* 拉低复位线进行复位 */
	OLED_RESET_START;
	OLED_Delay(40);
	OLED_RESET_END;
	OLED_Delay(40);
	/* 复位完成 */

	//	OLED19264_WRITE_COMMAND(0xFD);	//Command Lock  ( SSD1309 use, SSD1305 no use )
	//  OLED19264_WRITE_COMMAND(0x12);	// Unlock Driver IC (0x12/0x16)

	OLED19264_WRITE_COMMAND(0xAE); // Set Display Off

	OLED19264_WRITE_COMMAND(0xD5); //Set Display Clock Divide Ratio/Oscillator Frequency
	OLED19264_WRITE_COMMAND(0xD0); //	 1305:0x60~0x90(324~396KHz)	   1309: 0x70~0xd0(360~540KHz)

	OLED19264_WRITE_COMMAND(0xA8); //Set Multiplex Ratio
	OLED19264_WRITE_COMMAND(0x3F); //

	//	OLED19264_WRITE_COMMAND(0xD3);	//Set Display Offset
	//	OLED19264_WRITE_COMMAND(0x00);	//

	OLED19264_WRITE_COMMAND(0x40); // Set Display Start Line

	OLED19264_WRITE_COMMAND(0xA1); // Set SEG/Column Mapping (0xA0/0xA1)
	OLED19264_WRITE_COMMAND(0xC8); // Set COM/Row Scan Direction (0xC0/0xC8)

	OLED19264_WRITE_COMMAND(0xDA); //Set COM Pins Hardware Configuration
	OLED19264_WRITE_COMMAND(0x12); // Set Alternative Configuration (0x02/0x12)

	//	OLED19264_WRITE_COMMAND(0x81);	//Set Current Control
	//  OLED19264_WRITE_COMMAND(0xDF);  //

	OLED19264_WRITE_COMMAND(0xD9); //Set Pre-Charge Period
	OLED19264_WRITE_COMMAND(0x82); //

	OLED19264_WRITE_COMMAND(0xDB); //Set VCOMH Deselect Level
	OLED19264_WRITE_COMMAND(0x34); //

	//	OLED19264_WRITE_COMMAND(0x20);
	//	OLED19264_WRITE_COMMAND(0x02);	// Set Page Addressing Mode (0x00/0x01/0x02)

	//	OLED19264_WRITE_COMMAND(0xA4);	// Set Entire Display On/Off (0xA4/0xA5)
	//	OLED19264_WRITE_COMMAND(0xA6);	// Set Normal/Inverse Display (0xA6/0xA7)

	OLED_Delay(80);

	OLED19264_WRITE_COMMAND(0xAF); //Set Display On

	//Clear_Screen(); //清屏

	OLED_Delay(200);
}

/**
 *@brief 设置显示区域
 *@param 显示在page页
 *@param 从column列开始显示
 *@retval void
 */
void lcd_address(uint8_t page, uint8_t column)
{
	column = column - 1;
	page = page - 1;
	OLED19264_WRITE_COMMAND(0xb0 + page);
	OLED19264_WRITE_COMMAND(((column >> 4) & 0x0f) + 0x10);
	OLED19264_WRITE_COMMAND(column & 0x0f);
}

/**
 *@brief 清空全屏
 *@param void
 *@retval void
 */
void Clear_Screen(void)
{
	unsigned char i, j;
	for (i = 0; i < 8; i++)
	{
		lcd_address(1 + i, 1);
		for (j = 0; j < 128; j++)
		{
			OLED19264_WRITE_DATA(0x00);
		}
	}
}

/**
 *@brief 清空区域
 *@param 起始页，起始列，终止页，终止列
 *@retval void
 */
void Clear_Block(uint8_t page_start, uint8_t column_start, uint8_t page_end, uint8_t column_end)
{
	unsigned char i, j;
	for (i = page_start; i < page_end + 1; i++)
	{
		lcd_address(i, column_start);
		for (j = column_start; j < column_end + 1; j++)
		{
			OLED19264_WRITE_DATA(0x00);
		}
	}
}

/**
 *@brief 全屏显示
 *@param void
 *@retval void
 */
void full_display(void)
{
	int i, j;
	for (i = 0; i < 8; i++)
	{
		lcd_address(i + 1, 1);
		for (j = 0; j < 128; j++)
		{
			OLED19264_WRITE_DATA(0xff);
		}
	}
}

void Display_Num(uint8_t page, uint8_t column, uint8_t Number)
{
	char string[3];
	uint8_t op, tp, hp;
	op = Number / 1 % 10 + 0x30;
	tp = Number / 10 % 10 + 0x30;
	hp = Number / 100 % 10 + 0x30;

	string[0] = hp;
	if (hp == 0x30)
	{
		string[0] = 0x20;
	}
	string[1] = tp;
	string[2] = op;
	Display_String(page, column, string);
}

/**
 *@brief 在指定位置显示16x16中文字符
 *@param 显示在page页
 *@param 从column列开始显示
 *@param 字模数据
 *@retval void
 */
void display_graphic_16x16_cn(uint8_t page, uint8_t column, uint8_t *dp)
{
	uint8_t i, j;
	for (j = 0; j < 2; j++)
	{
		lcd_address(page + j, column);
		for (i = 0; i < 16; i++)
		{
			OLED19264_WRITE_DATA(*dp);
			dp++;
		}
	}
}

/**
 *@brief 在指定位置显示8x16英文字符
 *@param 显示在page页
 *@param 从column列开始显示
 *@param 字模数据
 *@retval void
 */
void display_graphic_8x16_en(uint8_t page, uint8_t column, uint8_t *dp)
{
	uint8_t i, j;
	for (j = 0; j < 2; j++)
	{
		lcd_address(page + j, column);
		for (i = 0; i < 8; i++)
		{
			OLED19264_WRITE_DATA(*dp);
			dp++;
		}
	}
}

/**
 *@brief 在指定位置通过字库显示字符串（只能显示中文、数字、英文、空格、和英文字符）
 *@param 显示在page页
 *@param 从column列开始显示
 *@param 字符串指针
 *@retval void
 */
void Display_String(uint8_t page, uint8_t column, char *p)
{
	column = column + 1;
	uint8_t *dp = (uint8_t *)p;
	uint8_t *read_temp_buffer = NULL;

	while (*dp != 0x00)
	{
		if (*dp > 0xa0) //汉字
		{
			switch (*dp)
			{
			case 0xB0:
				read_temp_buffer = gb2312_code + 14 * 32;
				break;
			case 0xB3:
				if (*(dp + 1) == 0xC9)
				{
					read_temp_buffer = gb2312_code + 11 * 32;
				}
				else if (*(dp + 1) == 0xCC)
				{
					read_temp_buffer = gb2312_code + 6 * 32;
				}
				break;
			case 0xB5:
				if (*(dp + 1) == 0xE7)
				{
					read_temp_buffer = gb2312_code + 59 * 16;
				}
				break;
			case 0xB6:
				if (*(dp + 1) == 0xAB)
				{
					read_temp_buffer = gb2312_code + 45 * 16;
				}
				break;
			case 0xB9:
				read_temp_buffer = gb2312_code + 12 * 32;
				break;
			case 0xBB:
				if (*(dp + 1) == 0xB6)
				{
					read_temp_buffer = gb2312_code + 35 * 16;
				}
				else if (*(dp + 1) == 0xAA)
				{
					read_temp_buffer = gb2312_code + 51 * 16;
				}
				else if (*(dp + 1) == 0xFA)
				{
					read_temp_buffer = gb2312_code + 49 * 16;
				}
				break;
			case 0xBD:
				read_temp_buffer = gb2312_code + 8 * 32;
				break;
			case 0xBF:
				if (*(dp + 1) == 0xF3)
				{
					read_temp_buffer = gb2312_code + 47 * 16;
				}
				else if (*(dp + 1) == 0xD8)
				{
					read_temp_buffer = gb2312_code + 61 * 16;
				}
				break;
			case 0xC2:
				read_temp_buffer = gb2312_code + 16 * 32;
				break;
			case 0xC4:
				if (*(dp + 1) == 0xDC)
				{
					read_temp_buffer = gb2312_code + 57 * 16;
				}
				break;
			case 0xC9:
				if (*(dp + 1) == 0xD5)
				{
					read_temp_buffer = gb2312_code + 15 * 32;
				}
				else if (*(dp + 1) == 0xBD)
				{
					read_temp_buffer = gb2312_code + 43 * 16;
				}
				break;
			case 0xCA:
				if (*(dp + 1) == 0xA7)
				{
					read_temp_buffer = gb2312_code + 13 * 32;
				}
				else if (*(dp + 1) == 0xD5)
				{
					read_temp_buffer = gb2312_code + 9 * 32;
				}
				else if (*(dp + 1) == 0xB9)
				{
					read_temp_buffer = gb2312_code + 39 * 16;
				}
				break;
			case 0xCD:
				if (*(dp + 1) == 0xB3)
				{
					read_temp_buffer = gb2312_code + 65 * 16;
				}
				else if (*(dp + 1) == 0xC5)
				{
					read_temp_buffer = gb2312_code + 69 * 16;
				}
				break;
			case 0xCF:
				if (*(dp + 1) == 0xB5)
				{
					read_temp_buffer = gb2312_code + 63 * 16;
				}
				break;
			case 0xD0:
				if (*(dp + 1) == 0xF2)
				{
					read_temp_buffer = gb2312_code + 7 * 32;
				}
				else if (*(dp + 1) == 0xC5)
				{
					read_temp_buffer = gb2312_code + 53 * 16;
				}
				break;
			case 0xD3:
				if (*(dp + 1) == 0xAD)
				{
					read_temp_buffer = gb2312_code + 37 * 16;
				}
				else if (*(dp + 1) == 0xC3)
				{
					read_temp_buffer = gb2312_code + 41 * 16;
				}
				break;
			case 0xD6:
				if (*(dp + 1) == 0xD0)
				{
					read_temp_buffer = gb2312_code + 10 * 32;
				}
				else if (*(dp + 1) == 0xC7)
				{
					read_temp_buffer = gb2312_code + 55 * 16;
				}
				break;
			case 0xBC:
				if (*(dp + 1) == 0xAF)
				{
					read_temp_buffer = gb2312_code + 67 * 16;
				}
				break;

			default:
				break;
			}
			display_graphic_16x16_cn(page, column, read_temp_buffer);
			column = column + 16;
			dp++;
		}
		else //ASCII
		{
			switch (*dp)
			{
			case 0x20:
				read_temp_buffer = space_code;
				break;
			case 0x30:
				read_temp_buffer = gb2312_code + 0 * 16;
				break;
			case 0x31:
				read_temp_buffer = gb2312_code + 1 * 16;
				break;
			case 0x32:
				read_temp_buffer = gb2312_code + 2 * 16;
				break;
			case 0x33:
				read_temp_buffer = gb2312_code + 3 * 16;
				break;
			case 0x34:
				read_temp_buffer = gb2312_code + 4 * 16;
				break;
			case 0x35:
				read_temp_buffer = gb2312_code + 5 * 16;
				break;
			case 0x36:
				read_temp_buffer = gb2312_code + 6 * 16;
				break;
			case 0x37:
				read_temp_buffer = gb2312_code + 7 * 16;
				break;
			case 0x38:
				read_temp_buffer = gb2312_code + 8 * 16;
				break;
			case 0x39:
				read_temp_buffer = gb2312_code + 9 * 16;
				break;
			case 0x25: //%
				read_temp_buffer = gb2312_code + 10 * 16;
				break;
			case 0x2E: //.
				read_temp_buffer = gb2312_code + 11 * 16;
				break;
			case 0x58: //X
				read_temp_buffer = gb2312_code + 34 * 16;
				break;

			default:
				break;
			}
			display_graphic_8x16_en(page, column, read_temp_buffer);
			column = column + 8;
		}
		dp++;
	}
}

void Display_Singal_Num(uint8_t page, uint8_t column, char Number)
{
	char unm[2];
	if (Number < 10)
	{
		unm[0] = 0x30;
		unm[1] = Number + 0x30;
	}
	if ((Number >= 10) && (Number <= 99))
	{
		unm[0] = Number / 10 + 0x30;
		unm[1] = Number % 10 + 0x30;
	}
	Display_String(page, column, unm);
}

void display_graphic_128x64(unsigned char page,unsigned char column,const unsigned char *dp)
{
	int i,j;
	for(j=0;j<2;j++)
	{
		lcd_address(page+j,column);
		for(i=0;i<16;i++)
		{
			OLED19264_WRITE_DATA(*dp);   
			dp++;
		}
	}          
}
