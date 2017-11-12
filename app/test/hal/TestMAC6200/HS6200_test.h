// HS6200 ���Գ���
// ***************************************************/
// // ע���� Keil �������Ŀ Target �����У��� C51 ѡ��
// // �������ð����ļ�Ŀ¼��


 #ifndef __HS6200_TEST_H__
 #define __HS6200_TEST_H__

extern U8 Cal_after_Ack;
extern U8 flush_tx_when_max_rt;

extern void nRF24L01_set_mode(U8 DevNum, rf_config_t NewConfig);
extern void System_Init(void);	  //�豸��ʼ��

// /**************************************************/

extern void nRF24L01_set_mode(unsigned char DevNum, rf_config_t NewConfig);  //24L01 set mode


//used for test
//Bank0�£�
extern void HS6200_Rst_Bank0_All_Register(U8 DevNum);
extern U8 HS6200_Read_Bank0_All_Register(U8 DevNum,U8 *Reg_Val);
extern void HS6200_Write1_to_Bank0_All_Register(U8 DevNum);
extern void HS6200_Write0_to_Bank0_All_Register(U8 DevNum);

extern U8 HS6200_Read_Bank1_All_Register(U8 DevNum,U8 *Reg_Val);
extern void HS6200_Rst_Bank1_All_Register(U8 DevNum);
extern U8 HS6200_Read_All_Register(U8 DevNum,U8* Reg_Val);

extern void HS6200_Mode_Config(U8 DevNum,U8 Config);

extern void HS6200_DEV0_Int(void);
extern void HS6200_DEV1_Int(void);



 #endif // __HS6200_TEST_H__


/*-----------------------------------End Of File---------------------------------------*/


