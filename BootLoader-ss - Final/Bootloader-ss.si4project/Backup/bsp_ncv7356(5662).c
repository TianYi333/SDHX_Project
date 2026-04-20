#include "bsp_ncv7356.h"
#include "bsp_oled19264.h"
#include "main.h"
#include "string.h"

extern uint8_t *CAN_Recieve_Buffer;
extern uint8_t Status_Flag;
extern uint32_t Program_Size;
extern volatile uint32_t Program_Size_Theoretical;
extern volatile uint32_t Program_Size_Actual;
extern uint8_t End_Flag;
extern uint8_t Trans_Flag;
uint8_t End_Buffer[8] = {0};

CAN_TxHeaderTypeDef CAN_TxHeader;

CAN_RxHeaderTypeDef CAN1_RxHeader;
CAN_RxHeaderTypeDef CAN2_RxHeader;
CAN_RxHeaderTypeDef CAN3_RxHeader;

uint8_t can1_recv_buf[8];
uint8_t can2_recv_buf[8];
uint8_t can3_recv_buf[8];
uint8_t can2_ask_buf[8] = {0x41,0x53,0x4B,0x49,0x53,0x4F,0x4B,0x21};


uint32_t Ret_MailBox = 0;

uint32_t Program_Size_nTemp = 0;
uint32_t Program_Size_nRmd = 0;


extern uint8_t Checksum_Value_recv;
extern uint8_t Checksum_Value;		
extern uint32_t nTemp_Value;		


uint8_t ntemp_can2_recv[512]={0};


void CAN_Config_Ready(void)
{
	CAN_FilterTypeDef CAN_FilterInitStructure;

	CAN_FilterInitStructure.FilterActivation = ENABLE;
	CAN_FilterInitStructure.FilterBank = 0;
	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	CAN_FilterInitStructure.FilterIdHigh = 0x0000 << 5;
	CAN_FilterInitStructure.FilterIdLow = 0x0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdHigh = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdLow = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN_FilterInitStructure.FilterScale = CAN_FILTERSCALE_16BIT;
	CAN_FilterInitStructure.SlaveStartFilterBank = 0;

	//HAL_CAN_ConfigFilter(&hcan1, &CAN_FilterInitStructure);
	//HAL_CAN_ConfigFilter(&hcan3, &CAN_FilterInitStructure);

	CAN_FilterInitStructure.FilterBank = 14;
	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO1;
	CAN_FilterInitStructure.FilterIdHigh = 0x0000 << 5; //0xA0程序传递准备过程的CAN ID
	CAN_FilterInitStructure.FilterIdLow = 0x0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdHigh = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdLow = 0X0000 << 5;
	CAN_FilterInitStructure.SlaveStartFilterBank = 14;
	HAL_CAN_ConfigFilter(&hcan2, &CAN_FilterInitStructure);

	//HAL_CAN_Start(&hcan1);
	HAL_CAN_Start(&hcan2);
	//HAL_CAN_Start(&hcan3);

	//HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);
	//HAL_CAN_ActivateNotification(&hcan3, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void CAN_Config_Trans(void)
{
	CAN_FilterTypeDef CAN_FilterInitStructure;

	CAN_FilterInitStructure.FilterActivation = ENABLE;
	CAN_FilterInitStructure.FilterBank = 0;
	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	CAN_FilterInitStructure.FilterIdHigh = 0x0000 << 5;
	CAN_FilterInitStructure.FilterIdLow = 0x0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdHigh = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdLow = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMode = CAN_FILTERMODE_IDMASK;
	CAN_FilterInitStructure.FilterScale = CAN_FILTERSCALE_16BIT;
	CAN_FilterInitStructure.SlaveStartFilterBank = 0;

	HAL_CAN_ConfigFilter(&hcan1, &CAN_FilterInitStructure);
	HAL_CAN_ConfigFilter(&hcan3, &CAN_FilterInitStructure);

	CAN_FilterInitStructure.FilterBank = 14;
	CAN_FilterInitStructure.FilterFIFOAssignment = CAN_FILTER_FIFO1;
	CAN_FilterInitStructure.FilterIdHigh = 0x0000 << 5; //0xA0程序传递过程的CAN ID
	CAN_FilterInitStructure.FilterIdLow = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdHigh = 0X0000 << 5;
	CAN_FilterInitStructure.FilterMaskIdLow = 0X0000 << 5;
	CAN_FilterInitStructure.SlaveStartFilterBank = 14;
	HAL_CAN_ConfigFilter(&hcan2, &CAN_FilterInitStructure);

	HAL_CAN_Start(&hcan1);
	HAL_CAN_Start(&hcan2);
	HAL_CAN_Start(&hcan3);

	HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
	HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO1_MSG_PENDING);
	HAL_CAN_ActivateNotification(&hcan3, CAN_IT_RX_FIFO0_MSG_PENDING);
}

uint8_t CANx_SendNormalData(CAN_HandleTypeDef *hcanX, uint16_t ID, uint8_t *pData, uint16_t Len)
{
	HAL_StatusTypeDef HAL_RetVal;
	uint16_t SendTimes, SendCNT = 0;
	CAN_TxHeader.StdId = ID;
	SendTimes = Len / 8 + (Len % 8 ? 1 : 0);
	CAN_TxHeader.DLC = 8;
	while (SendTimes--)
	{
		if (0 == SendTimes)
		{
			if (Len % 8)
				CAN_TxHeader.DLC = Len % 8;
		}
		
		while (HAL_CAN_GetTxMailboxesFreeLevel(hcanX) == 0)
			;

		HAL_Delay(1);

		HAL_RetVal = HAL_CAN_AddTxMessage(hcanX, &CAN_TxHeader, pData + SendCNT, &Ret_MailBox);
		if (HAL_RetVal != HAL_OK)
		{
			return 2;
		}
		SendCNT += 8;
	}
	return 0;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{

	if (hcan->Instance == CAN1)
	{
		HAL_CAN_GetRxMessage(&hcan1, CAN_FILTER_FIFO0, &CAN1_RxHeader, can1_recv_buf);
	}
	if (hcan->Instance == CAN3)
	{
		HAL_CAN_GetRxMessage(&hcan3, CAN_FILTER_FIFO0, &CAN3_RxHeader, can3_recv_buf);
	}
}
uint16_t pro_time;
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	if (hcan->Instance == CAN2)
	{
		
		HAL_CAN_GetRxMessage(&hcan2, CAN_FILTER_FIFO1, &CAN2_RxHeader, can2_recv_buf);
		if(CAN2_RxHeader.StdId == 0xA0)
		{
			if (Status_Flag == 0) //存储要接收的数据长度
			{
				if (can2_recv_buf[0] == 0xAA && can2_recv_buf[1] == 0xAA)
				{
					Program_Size_Theoretical = (can2_recv_buf[4] << 24) + (can2_recv_buf[5] << 16) + (can2_recv_buf[6] << 8) + (can2_recv_buf[7]);
					Status_Flag = 1;
					Program_Size_nTemp = Program_Size_Theoretical/8;
					Program_Size_nRmd = Program_Size_Theoretical%8;
				}
			}
		}
		else if (CAN2_RxHeader.StdId == 0xA1) //存储接收到的数据
		{
			Trans_Flag = 1; //开始接收标志位

			memcpy(CAN_Recieve_Buffer, can2_recv_buf, 8);

			if(Program_Size_nTemp)
			{
				Program_Size_nTemp--;
				for(int ii = 0;ii < 8;ii++)
				{
					nTemp_Value += can2_recv_buf[ii];
					nTemp_Value = nTemp_Value&0xFF;
					Checksum_Value = nTemp_Value;
				}
			}
			else
			{
				for(int ii = 0;ii < Program_Size_nRmd;ii++)
				{
					nTemp_Value += can2_recv_buf[ii];
					nTemp_Value = nTemp_Value&0xFF;
					Checksum_Value = nTemp_Value;
				}
				//CAN_Recieve_Buffer -= 504;
				//memcpy(ntemp_can2_recv, CAN_Recieve_Buffer, 512);
				//CAN_Recieve_Buffer -= 512;
			}

			CAN_Recieve_Buffer += 8;
			Program_Size_Actual += 8;


		}
		else if(CAN2_RxHeader.StdId == 0xA2)	//结束
		{
			Checksum_Value_recv = can2_recv_buf[6];
			Program_Size_Actual = Program_Size_Actual - 8 + can2_recv_buf[7];

			End_Flag = 1;
		}
	}
}
