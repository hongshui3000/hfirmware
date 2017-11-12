/*
 * HS6200���Ե�main�����ļ�
 *
 * ˵������STARTUP.A51�ļ������޸ģ�����line 112 and 134, �������ǹرտ��Ź�����ֹxdata̫��
 *       ����STARTUP.A51�ļ����г�ʼ��xdataʱʱ��̫�������µĿ��Ź���λ�������ǣ� 
 *       
 *       �ϵ�˳����HS6200/nRF24L01,Ȼ��MCU������ͬʱ�ϵ硣 ��Ϊ��LED Off��ϵͳ������裬��������
 *                 ȷ��������HS6200����nRF24L01,һ��MCU�ϹҽӼ���devive. 
 *
 * ���ã��������̵�������HS6200_Debug.h�ļ�	
 */


#include "hal.h"
#include "ch.h"
#include "nRF24L01_X.h"
#include "HS6200_test_sys.h"   
#include "HS6200_test.h"

#include "C8051F_USB.h"

#include "HS6200Test_Application_Protocol.h" 
#include "HS6200_Debug.h"
#include "HS6200_Analog_Test.h"
#include "stdlib.h"
#include "usbsubdev_hs6200.h"


U8 g_interrupt_flag=INTERRUPT_NONE;

//static void HS6200_interrupt_disable(void)
//{
//  HS_GPIO_Type *pGpio1 = IOPORT1;
//  pGpio1->INTENCLR |= BIT5;  
//}
//
//static void HS6200_interrupt_able(void)
//{
//  HS_GPIO_Type *pGpio1 = IOPORT1;
//  pGpio1->INTENSET |= BIT5;   
//}
/*---------------------------------------------------Interrupt Service Routine-------------------------------------------------*/

/*****************************************************
Function: ISR_int0() interrupt 0;
 
Description:
if RX_DR=1 or TX_DS or MAX_RT=1,enter this subprogram;
if RX_DR=1,read the payload from RX_FIFO and set g_flag;
**************************************************/
void ISR_int0(void) //interrupt 0          // nRF24L01_0 �ж�
{
  HS_GPIO_Type *pGpio1 = IOPORT1;
  g_interrupt_flag = INTERRUPT_HS6200;
//  HS6200_interrupt_disable();
  pGpio1->INTSTATUS = pGpio1->INTSTATUS;
}

void HS6200_interrupt_process(void) 
{
	LED1_ON;
    g_interrupt_flag = INTERRUPT_NONE;
	HS6200_Bank0_Activate(DEV_1);
	HS6200_DEV1_Int();
    LED1_OFF;
}


/**************************************************/

/**************************************************
Function: ISR_int1() interrupt 2;
 
Description:
if RX_DR=1 or TX_DS or MAX_RT=1,enter this subprogram;
if RX_DR=1,read the payload from RX_FIFO and set g_flag;
**************************************************/
void ISR_int1(void) 
{
  g_interrupt_flag = INTERRUPT_MAC6200;
}

void MAC6200_interrupt_process(void) 
{
	LED2_ON;
    MAC6200_CE_Low();
    g_interrupt_flag = INTERRUPT_NONE;
	HS6200_Bank0_Activate(DEV_1);
	HS6200_DEV1_Int();
    MAC6200_CE_High(); 
    LED2_OFF;
}

/**************************************************/
// ISR for USB_API
void usb_recevice_process(void)//INTERRUPT(USB_API_TEST_ISR, INTERRUPT_USBXpress)
{
  U8 Temp_Buf[MAX_OUT_PACKET_LENGTH];
  U8 Temp_Buf_Length;
  
  if( (USB_Host_Out_Flag==USB_HOST_OUT_FLAG_C8051F_DISCARD) || (USB_Host_Out_Flag==USB_HOST_OUT_FLAG_C8051F_USED) )  //�����Ѿ�ʹ��
  {
    USB_Host_Out_Packet_Length=Block_Read(USB_Host_Out_Packet, MAX_OUT_PACKET_LENGTH);  //read USB packet data
    USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_RECEIVED;  //���ݽ��յ�
    
    //�жϴ�ʱ���͵������Ƿ���ֹͣTest Pattern
    //���
    //0A xx	08 02 -->AT_CTT_TESTPAT_STOP
    if( (0x0A==USB_Host_Out_Packet[0]) && (0x02==USB_Host_Out_Packet[2]) && (0x08==USB_Host_Out_Packet[3]) &&  (0x02==USB_Host_Out_Packet[4]) )
    {
      if(DEV_0==USB_Host_Out_Packet[1])        //DEV_0
      {
        Test_Pattern_Psdo_Rdm_Num_Flag[DEV_0]=TEST_PAT_PSDO_RDM_NUM_FLG_STOP;	  //stop pseudo random number test
      }
    else if(DEV_1==USB_Host_Out_Packet[1])	 //DEV_1
      {
        Test_Pattern_Psdo_Rdm_Num_Flag[DEV_1]=TEST_PAT_PSDO_RDM_NUM_FLG_STOP;	  //stop pseudo random number test
      }
    }
    else if( (0x0E==USB_Host_Out_Packet[0]) && (0x01==USB_Host_Out_Packet[2]) && (USB_Host_Out_Packet[3]==DEBUG_CE_HIGH_LOW_STOP))  //debug stop
      {
        Debug_ce_High_Low_Stop_Flag=0x01;
      } 
    }
  else 
  {
    Temp_Buf_Length=Block_Read(Temp_Buf,MAX_OUT_PACKET_LENGTH);
    USB_Discard_Dev_Num=Temp_Buf[2];
    USB_Host_Out_Flag=USB_HOST_OUT_FLAG_C8051F_DISCARD;  //���ݶ���
  }
}

void usb_Transmitted_process(void)
{
  USB_Host_In_Flag=USB_HOST_IN_FLAG_COMPLETED;  
}

/*---------------------------------------------------------------------End Of File---------------------------------------------------------------------*/


