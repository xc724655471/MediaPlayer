#pragma once

/**
* DecodePacket
* //�ڲ��ṹ�塣�ýṹ����Ҫ���ڴ洢�ʹ���Nal��Ԫ�����͡���С������
*/
typedef struct DecodePacket
{
	//int type;
	int size;
	unsigned char *cur_ptr;
}DecodePacket;


/**
* PlayPacket
* //�ڲ��ṹ�塣�ýṹ����Ҫ���ڴ洢�ʹ���Nal��Ԫ�����͡���С������
*/
typedef struct PlayPacket
{
	unsigned char *yuvdate;
	int yuvlinesize;
	float time;
}PlayPacket;



