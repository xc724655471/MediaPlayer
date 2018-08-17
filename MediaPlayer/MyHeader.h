#pragma once

/**
* DecodePacket
* //内部结构体。该结构体主要用于存储和传递Nal单元的类型、大小和数据
*/
typedef struct DecodePacket
{
	//int type;
	int size;
	unsigned char *cur_ptr;
}DecodePacket;


/**
* PlayPacket
* //内部结构体。该结构体主要用于存储和传递Nal单元的类型、大小和数据
*/
typedef struct PlayPacket
{
	unsigned char *yuvdate;
	int yuvlinesize;
	float time;
}PlayPacket;



