/*
 * infrared_key_info.h
 *
 *  Created on: Mar 8, 2017
 *      Author: woshi
 */

#ifndef INFRARED_KEY_INFO_H_
#define INFRARED_KEY_INFO_H_

#if HAL_USE_IR

#include "lib.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include "lib_infrared.h"

/*
 *  ICU Driver1  : Timer1  PA6 PA7(channel_1/2)
 *  ICU Driver2  : Timer2  PB5 PB4(channel_1/2)
 */

uint8_t  IR_pin = PA6;  //Default ICU_PIN
uint8_t  long_press_effect = 3; // ���������ȣ�ԽСԽ����

int nec_dict_len = 21;  // ��󰴼���Ŀ
nec_key_info_t nec_key_dictionary_0[]=
{
		{0,   48,  "0",   0},
		{1,   8,   "1",   0},
		{2,   216,  "2",   0},
		{3,   248,  "3",   0},
		{4,   48,   "4",   0},
		{5,   176,  "5",   0},
		{6,   112,  "6",   0},
		{7,   0,    "7",   0},
		{8,   240,  "8",   0},
		{9,   152,  "9",   0},
		{10,  80,   "funset",   0},
		{11,  88,   "mode",   0},
		{12,  120,  "mute",   0},
		{13,  128,  "pause",   0},
		{14,  192,  "prev",   0},
		{15,  64,   "next",   0},
		{16,  32,   "eq",   0},
		{17,  160,  "vol-",   1},
		{18,  96,   "vol+",   1},
		{19,  16,   "rpt",   0},
		{20,  144,  "usd",   0},
};


nec_key_info_t nec_key_dictionary_97[]=
{
		{0,   224,  "0",   0},
		{1,   255,  "1",   0},
		{2,   216,  "2",   0},
		{3,   248,  "3",   0},
		{4,   48,   "4",   0},
		{5,   176,  "5",   0},
		{6,   112,  "6",   0},
		{7,   0,    "7",   0},
		{8,   240,  "8",   0},
		{9,   152,  "9",   0},
		{10,  72,   "power",   0},
		{11,  88,   "mode",   0},
		{12,  120,  "mute",   0},
		{13,  128,  "pause",   0},
		{14,  192,  "prev",   0},
		{15,  64,   "next",   0},
		{16,  32,   "eq",   0},
		{17,  160,  "vol-",   1},
		{18,  96,   "vol+",   1},
		{19,  16,   "rpt",   0},
		{20,  144,  "usd",   0},
};

int nec_dev_num = 2;   //��֧�ֵ�ͬʱ���ߵ�ң���豸��Ŀ
nec_dev_dict_t nec_dev_dict[]=
{
		{0,nec_key_dictionary_0},
		{97,nec_key_dictionary_97},
};



/*
 *  ��������Ϣ  ��Ҫ����config.ini������Ч
 *  sub_mesg_index,sub_cmd, profile, status
	30, 00, 0001, 000f //enter_pair,     gap, null|ready|pairable|paired
	31, 01, 0001, ffff //clr_pair_list,  gap, any
	32, 48, 0001, 000f //connect_back,   gap, null|ready|pairable|paired
	33, 49, 0001, 0030 //disconnect_all, gap, connected_idle|calling
	42, 25, 0002, 000a //play,     a2dp, open|pause
	42, 26, 0002, 0004 //pause,    a2dp, start
	48, 29, 0002, 000C //next,  a2dp, start
	49, 2a, 0002, 000C //prev,  a2dp, start
	43, 04, 0004, 0010 //reject,  hfp, incoming
	43, 05, 0004, 000C //hangup,  hfp, ongoing/outgoing
	43, 06, 0001, 0010 //redial,  gap, connected_idle
	43, 05, 0001, 0020 //hangup,  gap, connected_calling
	43, 06, 0002, 001f //redial
	44, 03, 0004, 0010 //answer,  hfp, incoming
	44, 20, 0004, 0004 //sco connect/disconect,  hfp, ongoing/outgoing
	48, 3a, 0010, 0002 //hid sendkey1
	49, 3b, 0010, 0002 //hid sendkey2
	40, 4a, 0001, 00FF //vol_add,  a2dp, start
	41, 4b, 0001, 00FF //vol_sub,  a2dp, start
 */

uint16_t nec_event_cnt = 23;        // ֧�ֵ�IR��Ϣ�������
ir_sys_mesg_t ir_sys_mesg_dict[]=
{
		//  ϵͳ��   1

		//  ������  4    /9
		{"pause",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x42},    // ������ͣ�벥��
		{"volinc",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x40},   // ������
		{"voldec",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x41},   // ������
		{"next",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x48},     // ��һ��
		{"prev",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x49},     // ��һ��
		{"reject",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x43},   // �ܽ�
		{"accept",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x44},   // ����
		{"end",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x43},      // ����
		{"recall",0,HS_CFG_MODULE_BT_CLASSIC,HS_CFG_EVENT_BT_CONTROL_CMD,0x43},   // �ز�

		//��������   5   /13
		{"stop",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_STOP,0},      // ������ͣ
		{"pause",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_START,0},    // ������ͣ�벥��
		{"volinc",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_VOLINC,0},   // ������
		{"voldec",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_VOLDEC,0},   // ������
		{"next",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_NEXT,0},     // ��һ��
		{"prev",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_PREV,0},     // ��һ��
		{"mute",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_MUTE,0},     // ����
		{"fminc",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_FREQINC,0},    // FM��Ƶ����
		{"fmdec",0,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_FREQDEC,0},    // FM��Ƶ��С
		{"funset",1,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_FUNCSET,0},      // 0 ѭ��ģʽ֮���л�
		{"funset",1,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_FUNCSET,0},      // 1 ��Ч֮���л�
		{"funset",1,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_FUNCSET,0},     //  0 FM��̨���洢
		{"funset",1,HS_CFG_MODULE_PLAYER,HS_CFG_EVENT_PLAYER_FUNCSET,0},      // 1 FM��մ洢��̨

		//�����    2

		//�û�1�Զ���� 8   / 1
		{"mode",0, HS_CFG_MODULE_USER1,HS_CFG_EVENT_MODE_CHANGE,0}, // ģʽ�л�
};




#endif


#endif /* INFRARED_KEY_INFO_H_ */
