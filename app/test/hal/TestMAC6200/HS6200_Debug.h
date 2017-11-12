
/*
 *  HS6200���Թ��̵�����˵����
 *  1.���̵Ĵ��������Ϣ�ǿ���ͨ���꣨macro�������Ƿ������HS6200_DEBUG macro�����𵽿��ƴ�����Ϣ�Ƿ����
 *	2.DISP_OUT_ERR macro ���ƴ�������ķ�ʽ��һ��ͨ��LED�����һ��ͨ��USB�������
 *  3.HS6200_DEVx ����DEV_x��HS6200����nRF24L01,
 */


#ifndef HS6200_DEBUG_H
#define HS6200_DEBUG_H

#include "nRF24L01_X.h"

extern void LED_Out_Err_Info(U8 Err_Info);
extern void USB_Out_Err_Info(U8 DevNum,U8 Err_Info); 
extern void USB_ACK_Diagnose_Info(U8 DevNum);
 

//global config
 
//output error number by LED������ʱ��ͬʱ���HS6200_test_sys.c�ļ��µ�Port_IO_Init()���������޸�
#define DISP_OUT_ERR	   P4    
#define LED_OUT_ERR        0x01
#define USB_OUT_ERR        0x10



#define HS6200_DEBUG			 //�����Ƿ����������Ϣ
#define DEBUG_OUT_ERR   USB_OUT_ERR   	 //����ͨ�����ַ�ʽ���������Ϣ

//���Debug����
#define DBG_ON       0x01
#define DBG_OFF      0x00 





/*-------------------------------------------------Err Information Macro----------------------------------------------------------*/
#define ERR_USB_PACKET_FUN_NOT_FINSH         0xFF	 //��ʾ�ù���û��ʵ��


//define err infomation
#define ERR_USB_PACKET_PID				     0x01

// Communication err
//#define ERR_USB_PACKET_COMM				           0x02
#define ERR_USB_PACKET_COMM_CMD              0x10 
#define ERR_USB_PACKET_COMM_RF_DEV           0x11
#define ERR_USB_PACKET_COMM_LENGTH           0x12
#define ERR_USB_PACKET_COMM_PIPE             0x13
#define ERR_USB_PACKET_COMM_DATA             0x14

//setup err
#define ERR_USB_PACKET_SETUP_CMD             0x20              /*LSBit may be 0*/
#define ERR_USB_PACKET_SETUP_RF_DEV          0x21
#define ERR_USB_PACKET_SETUP_LENGTH          0x22
#define ERR_USB_PACKET_SETUP_DATA            0x23

//SPI err
#define ERR_USB_PACKET_SPI_CMD               0x30
#define ERR_USB_PACKET_SPI_RF_DEV            0x31
#define ERR_USB_PACKET_SPI_LENGTH            0x32
#define ERR_USB_PACKET_SPI_DATA              0x33

//spec err
#define ERR_USB_PACKET_SPEC_CMD              0x40
#define ERR_USB_PACKET_SPEC_RF_DEV           0x41
#define ERR_USB_PACKET_SPEC_LENGTH           0x42
#define ERR_USB_PACKET_SPEC_DATA             0x43  

//ditgital test err
#define ERR_USB_PACKET_DT_CMD                0x90
#define ERR_USB_PACKET_DT_RF_DEV             0x91
#define ERR_USB_PACKET_DT_LENGTH             0x92
#define ERR_USB_PACKET_DT_DATA               0x93
   
//analog test err
#define ERR_USB_PACKET_AT_CMD                0xA0
#define ERR_USB_PACKET_AT_RF_DEV             0xA1
#define ERR_USB_PACKET_AT_LENGTH             0xA2
#define ERR_USB_PACKET_AT_DATA               0xA3



//set err
#define ERR_USB_PACKET_SET_CMD               0xC0
#define ERR_USB_PACKET_SET_RF_DEV            0xC1
#define ERR_USB_PACKET_SET_LENGTH            0xC2
#define ERR_USB_PACKET_SET_DATA              0xC3

//get err
#define ERR_USB_PACKET_GET_CMD               0xD0
#define ERR_USB_PACKET_GET_RF_DEV            0xD1
#define ERR_USB_PACKET_GET_LENGTH            0xD2
#define ERR_USB_PACKET_GET_DATA              0xD3

//debug err
#define ERR_USB_PACKET_DBG_CMD               0xE0
#define ERR_USB_PACKET_DBG_RF_DEV            0xE1
#define ERR_USB_PACKET_DBG_LENGTH            0xE2
#define ERR_USB_PACKET_DBG_DATA              0xE3


//#define ERR_NRF24L01_0_PLOS                  0x26
//#define ERR_NRF24L01_1_PLOS                  0x27  
//#define ERR_NRF24L01_0_RX_PAYLOAD_WIDTH      0x28
//#define ERR_NRF24L01_0_RX_PAYLOAD_CONTEXT    0x29
//#define ERR_NRF24L01_1_RX_PAYLOAD_WIDTH      0x2A
//#define ERR_NRF24L01_1_RX_PAYLOAD_CONTEXT    0x2B 

//--------------------------���������Ϣ-----------------------------------
extern U8 SoftWare_Date[6];  //����
extern U8 SoftWare_Ver;      //�汾��
extern U8 Debug_ce_High_Low_Stop_Flag;

#define DEBUG_SOFTWARE_DATE                    0x01    //ֻ��
#define DEBUG_SOFTWARE_VER                     0x02    //ֻ�� 
#define DEBUG_DEVICE_FLG                       0x03    //���ֶ�����Dev_Flag
#define DEBUG_MCU_INT                          0x04    //ֻ������ͨ����λ���� ���޸���ֵ
#define DEBUG_DEV_CE                           0x05    //ֻ������ͨ����λ���� ���޸���ֵ
#define DEBUG_CAL_AFTER_ACK                    0x06    //ֻ������ͨ����λ���� ���޸���ֵ
#define DEBUG_CE_LOW_BEFORE_WRITE              0x07    //ֻ������ͨ����λ���� ���޸���ֵ
#define DEBUG_DEV_BUSY                         0x08    //nRF24L01_X_Busy[]������
#define DEBUG_CE_STATE_BEFORE_FLUSH            0x09

#define DEBUG_READ_RX_PAYLOAD                  0x0A 
#define DEBUG_READ_ADDR                        0x0B
#define DEBUG_CE_HIGH_LOW_START                0x0C
#define DEBUG_CE_HIGH_LOW_STOP                 0x0D


#define DEBUG_TEST_MODE_OTHER_SPI_WRITE        0xA0    //���ڲ���ģʽ              

#endif  /*HS6200_DEBUG_H*/

/*---------------------------------End Of File----------------------------------------*/
