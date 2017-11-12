/*
 * lib_infrared.h
 *
 *  Created on: Mar 9, 2017
 *      Author: woshi
 */

#ifndef LIB_INFRARED_H_
#define LIB_INFRARED_H_

#ifdef __cplusplus
extern "C" {
#endif

#if HAL_USE_IR


//  ��������
typedef enum
{
  NEC_SIGNAL_PRESS = 0,
  NEC_LONG_PRESS
}nec_key_type_t;

// ��������(����      ����ֵ    ��    �Ƿ�֧�ֳ���)
typedef struct
{
	uint8_t key_index;
	uint8_t key_data;
	char    key_name[7];
	uint8_t spt_longpress;
}nec_key_info_t;


//ң����֧���б��豸������ַ   �������Ա�
typedef struct
{
	uint8_t dev_address;
	nec_key_info_t *nec_key_dictionary;
}nec_dev_dict_t;


//����ң�ذ�����Ϣ (��������      �˴ΰ���������(����/����))
typedef struct
{
	nec_key_info_t     nec_key;
	nec_key_type_t     signal_long;
}nec_mesg_t;


//  IR��Ϣ�ṹ
typedef struct
{
	char       key_name[7];        // ������
	uint8_t    special_arg_cnt;    // ������Ҫ�Ĳ�����Ŀ����С��Ч��Ŀ��
	uint8_t    u8EventMod;         // ��Ϣ������  ..... ��Ϣ����Ĭ��HS_CFG_SYS_EVENT
	uint16_t   u16Event;           // ��Ϣkeyֵ
	uint32_t   u32Arg;             // ��Ϣ���Ӳ���
}ir_sys_mesg_t;

void hs_infrared_init(void);
void hs_infrared_uninit(void);
void hs_user_handle(void *parg);
#endif


#ifdef __cplusplus
}
#endif


#endif /* LIB_INFRARED_H_ */
