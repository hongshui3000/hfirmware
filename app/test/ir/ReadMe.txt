<...........................
������ʹ�õ���PA6  Timer1
............................>



1��lib\include\halconf.h     HAL_USE_IR   �� ��Ҫʹ�ú���ң�ع�������halconf.h �ļ����Ƶ� HAL_USE_IR ΪTRUE

2��lib\include\config\config.h       �� �����Ϣ����

                      #if  HAL_USE_IR
  				HS_CFG_EVENT_IR_INFO                              = 0x107,
 			 	HS_CFG_EVENT_MODE_CHANGE                          = 0x108,
		      #endif

3��os\hal\board\HUNTERSUN_HS6601\ ... board.h     HS_IR_INPUT   :  ����IR�ź�����

4����main��������� 
#if HAL_USE_IR
#include"lib_infrared.h"
#endif

------

#if HAL_USE_IR
  hs_infrared_init();
#endif

5����app\test\cmd.c�����

#if HAL_USE_IR
extern void cmd_nec(BaseSequentialStream *chp, int argc, char *argv[]);
#endif


6��lib\include\mcuconf.h

#define HS_PWM_USE_TIM0           TRUE
#define HS_PWM_USE_TIM1           TRUE
#define HS_ICU_USE_TIM2           TRUE
     
     ICU Driver1  : Timer1  PA6 PA7(channel_1/2)
     ICU Driver2  : Timer2  PB5 PB4(channel_1/2)


7���������

�޸�IR����ܽ� ������ ��ʹ�õ� ICU_Timer���Ӧ   

8�� �û�ֻ����infrared_key_info.h����infrared_interface.c����ʵ���������ܵ��޸ġ�������