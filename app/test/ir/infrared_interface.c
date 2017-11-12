/*
 * infrared_interface.c
 *
 *  Created on: May 27, 2017
 *      Author: woshi  duo yuntao
 */

#include "lib.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "infrared_key_info.h"
#define REF 1 // �˿��ƵĴ����Ƕ�ң����Ϣ�Ľ��պͺ������Ƶ�һ�ֲο�,�û���������������ʺ�����ķ�ʽ

#if HAL_USE_IR
#include "lib_infrared.c"

#if REF != 1
void hs_user_handle(void *parg)
{
	nec_mesg_t *nec_mesg_parg = (nec_mesg_t *)parg;
	/*
	 *    �û�����յ���Ϣ  ��Ϣ�ṹ�μ�"lib_infrared.h" �е� nec_mesg_t
	 *    ������Ϣ��name���û����Խ���������������
	 *    {
	 *      nec_key_info_t     nec_key;
	 *      nec_key_type_t     signal_long;
	 *    }
	 *    EXP ��
	 *    if      nec_mesg_parg->nec_key.key_name = "stop"
	 *    then    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_STOP)
	 */


	if(nec_mesg_parg!=NULL)
	{
		//�ڴ�����û���������


   }
}
#else

typedef struct
{
	uint8_t key_index;
	uint8_t special_arg_cnt;
}key_index_argcnt_t;
static bool need_ir_arg = false;   // true���ܲ���״̬      false ����Ҫ����������Ѿ��������
static bool prepare_recv = false;  // �Ƿ�׼�������ղ���
static key_index_argcnt_t key_index_argcnt[5]={};  // ͬ���İ������������Լ����������Ŀ
static uint8_t  special_arg_cnt_kind = 0;  // ����¼5��ͬ���������������Ŀ
static uint8_t  special_arg_cnt_kind_min = 6; // ��С��Ч������Ŀ(������5������)
static uint8_t  special_arg_cnt_kind_max = 0; // ��С��Ч������Ŀ(������5������)
static uint8_t  recv_arg_cnt = 0;   // ��õĲ�����Ŀ
static uint8_t *arg = NULL, *base_arg = NULL;
static uint8_t warning_mask = 0;  // ����
static os_timer_type nec_arg_timer_type=osTimerOnce;   // ���ζ�ʱ��
static osTimerId nec_arg_TimerID=NULL;
static osTimerDef_t nec_arg_stTmDef;
static uint32_t wait_arg_time = 2000;  // 2s�Ĳ����ȴ�ʱ��


static void _timer_arg(void const *arg)     // ��ʱ��������
{
	 (void)arg;
	 need_ir_arg = false;
	 hs_user_handle((void*)(uint32_t)(-1));
}

void hs_user_handle(void *parg)
{
	nec_mesg_t *nec_mesg_parg = (nec_mesg_t *)parg;
	/*
	 *    �û�����յ���Ϣ  ��Ϣ�ṹ�μ�"lib_infrared.h" �е� nec_mesg_t
	 *    ������Ϣ��name���û����Խ���������������
	 *    {
	 *      nec_key_info_t     nec_key;
	 *      nec_key_type_t     signal_long;
	 *    }
	 *    EXP ��
	 *    if      nec_mesg_parg->nec_key.key_name = "stop"
	 *    then    hs_cfg_playerReq(HS_CFG_EVENT_PLAYER_STOP)
	 */

	// ----------------------------------���²��ֽ����ο�------------------------------------------


	int i,j;
	if(nec_mesg_parg!=NULL )
	{

		// --------- �����ڴ��Լ���ʱ���γ�ʼ�� --------
		if(base_arg==NULL)
		{
			base_arg = hs_malloc(sizeof(uint8_t)*5, __MT_Z_GENERAL);
			if(base_arg==NULL)
			{
				hs_printf("Not enough memory for special_arg\r\n");
				return;
			}
		}
		if(nec_arg_TimerID==NULL)
		{
			// Create Timer
			nec_arg_stTmDef.ptimer = _timer_arg;
			nec_arg_TimerID = oshalTimerCreate(&nec_arg_stTmDef,nec_arg_timer_type, NULL);

			if(nec_arg_TimerID==NULL)
			{
				hs_printf("Create Timer Error\r\n");
				return;
			}
		}
		// --------------------------------------------
		if((uint32_t)nec_mesg_parg == (uint32_t)(-1))
			goto SEND;
		/*
		 *  a �е������Ҫ����������յ�֮��ֱ�ӷ�����Ϣ��
		 *  b �е�������Ҫ���������ܻ���ֹһ��������һ����ʱ����ͨ��ң������ȡ�㹻�Ĳ������������Ϣ
		 *     b.0�����趨��ʱ����û�л���㹻�ĺϷ�������ʱ����������������
		 *     b.1 �������ʱ��  ������һ������,(��ʱ�����¼�ʱ)��ʼ������һ������;
		 *     b.3 Ŀǰ����ֻ����0-9������
		 */

		// @ 1 ƥ�䰴���������ֵ�����,����ȡ����Ҫ��special������Ŀ

		if(need_ir_arg != true || special_arg_cnt_kind == 0)
		{
			//  δ������������֮ǰ������0-9����    ��   ����Ҫ����
			if(nec_mesg_parg->nec_key.key_index<=9)
			{
				//  û�кϷ��ĵõ��Ϸ��Ĳ����� ���� û������������֮ǰ����������0-9
				warning_mask |= 0x02;
				goto END;
			}
		}


		for(i=0;i<nec_event_cnt;i++)
		{
			if(nec_mesg_parg->nec_key.key_index<=9)
				break;
			if(i==0)
			{
				// �յ�����������������ƥ��
				special_arg_cnt_kind = 0;     // ����¼5��ͬ���������������Ŀ
				special_arg_cnt_kind_min = 6; // ��С��Ч������Ŀ(������5������)
				special_arg_cnt_kind_max = 0; // �����Ч������Ŀ(������5������)
				recv_arg_cnt = 0;   // ��õĲ�����Ŀ
				need_ir_arg=false;  // Ĭ�ϲ���Ҫ����
				memset(key_index_argcnt,0,sizeof(key_index_argcnt_t)*5);
				warning_mask = 0;
			}
			if(strcmp(nec_mesg_parg->nec_key.key_name,ir_sys_mesg_dict[i].key_name)==0)
			{
				key_index_argcnt[special_arg_cnt_kind].key_index = i;
				key_index_argcnt[special_arg_cnt_kind].special_arg_cnt = ir_sys_mesg_dict[i].special_arg_cnt;
				if(special_arg_cnt_kind_min>ir_sys_mesg_dict[i].special_arg_cnt)
					special_arg_cnt_kind_min = ir_sys_mesg_dict[i].special_arg_cnt;
				if(special_arg_cnt_kind_max<ir_sys_mesg_dict[i].special_arg_cnt)
					special_arg_cnt_kind_max = ir_sys_mesg_dict[i].special_arg_cnt;
				special_arg_cnt_kind += 1;
				if(special_arg_cnt_kind>=5)
				{
					hs_printf("Five operations of the same name are supported at most\r\n");
					break;
				}

			}
			if(i==nec_event_cnt-1)
			{
				if( special_arg_cnt_kind==0 )
				{
					//  û��ƥ��Ϸ��Ĳ���
					warning_mask |= 0x01;
					goto END;
				}
				else
				{
					if(special_arg_cnt_kind_max!=0)
					{
						//  ��Ҫ�������
						need_ir_arg = true;
					}
				}
			}
		}



		// @ 2 ������Ϣ
		//      2.1��Ҫ�����������
		if(need_ir_arg==true)
		{
			if(prepare_recv == false)
			{

				//  ׼������
				arg = base_arg;
				oshalTimerStop(nec_arg_TimerID);
				oshalTimerStart(nec_arg_TimerID,wait_arg_time);
				prepare_recv = true;

			}
			else
			{
				// ���ղ���Ų���
				if(nec_mesg_parg->nec_key.key_index<=9)
				{
					*arg = nec_mesg_parg->nec_key.key_index;
					arg++;
					hs_printf("Receive : %d\r\n",*(arg-1));

					recv_arg_cnt+=1;
					if(recv_arg_cnt<special_arg_cnt_kind_max)
					{
						//  ������һ������
						oshalTimerStop(nec_arg_TimerID);
						oshalTimerStart(nec_arg_TimerID,wait_arg_time);
					}
					else
					{
						// �������������Ҫ�Ĳ�������
						hs_printf("The special_args is full\r\n");
						need_ir_arg = false;
						oshalTimerStop(nec_arg_TimerID);
					}
				}

			}

		}


		// 2.2  �����Ϸ�����Ҫ��������ʼ������Ϣ
SEND:
		if(need_ir_arg==false && special_arg_cnt_kind!=0)
		{
			// ���ݽ��յ���������Ŀ�����ж�Ӧ����
			if(recv_arg_cnt<special_arg_cnt_kind_min)
			{
				warning_mask |= 0x04;
				goto END;
			}
			hs_printf("@Sending.......\r\n\n");
            for(i=0;i<special_arg_cnt_kind;i++)
            {
            	if(key_index_argcnt[i].special_arg_cnt==recv_arg_cnt)
            	{
            		 j = key_index_argcnt[i].key_index;
            		 if(j>=18&&j<=21)
            		 {
            			 hs_cfg_sysSendMsg((hs_cfg_mod_t)ir_sys_mesg_dict[j].u8EventMod, HS_CFG_SYS_EVENT,
											(hs_cfg_event_type_t)(ir_sys_mesg_dict[j].u16Event | FAST_EVENT_MASK),
											(void*)((uint32_t)(*base_arg)));
            		 }
            		 else
            		 {
            			 hs_cfg_sysSendMsg((hs_cfg_mod_t)ir_sys_mesg_dict[j].u8EventMod, HS_CFG_SYS_EVENT,
											(hs_cfg_event_type_t)(ir_sys_mesg_dict[j].u16Event | FAST_EVENT_MASK),
											(recv_arg_cnt==0?(void *)ir_sys_mesg_dict[j].u32Arg:(void*)base_arg));

            		 }


            	}

            }

			special_arg_cnt_kind = 0;  // ����¼5��ͬ���������������Ŀ
			special_arg_cnt_kind_min = 6; // ��С��Ч������Ŀ(������5������)
			special_arg_cnt_kind_max = 0; // ��С��Ч������Ŀ(������5������)
			recv_arg_cnt = 0;   // ��õĲ�����Ŀ
			prepare_recv = false;
			memset(key_index_argcnt,0,sizeof(key_index_argcnt_t)*5);
			warning_mask = 0;
		}


 END:
	 if((warning_mask & 0x01) != 0 )
	 {

		 //  û�ж�Ӧ�ĺϷ�����
		 hs_printf("........ Warning 0x01 .......\r\n");
		 hs_printf("No legal operation\r\n\n");
		 warning_mask &= ~(0x01);
	 }
	if((warning_mask & 0x02) != 0 )
	 {
		 hs_printf("........ Warning 0x02.......\r\n");
		 hs_printf("Please input operation before digital_args\r\n");
		 hs_printf("If the operation need digital_args please input in given time\r\n\n");
		 warning_mask &= ~(0x02);
	 }
	 if((warning_mask & 0x04)!=0)
	 {
		 hs_printf("........ Warning 0x04.......\r\n");
		 hs_printf("Illegal arg_number\r\n\n");
		 warning_mask &= ~(0x04);
		 special_arg_cnt_kind = 0;  // ����¼5��ͬ���������������Ŀ
		 special_arg_cnt_kind_min = 6; // ��С��Ч������Ŀ(������5������)
		 special_arg_cnt_kind_max = 0; // ��С��Ч������Ŀ(������5������)
		 recv_arg_cnt = 0;   // ��õĲ�����Ŀ
		 prepare_recv = false;
		 memset(key_index_argcnt,0,sizeof(key_index_argcnt_t)*5);
	 }
	}
}


#endif    //  end REF

#endif    //  end HS_IR_USE





