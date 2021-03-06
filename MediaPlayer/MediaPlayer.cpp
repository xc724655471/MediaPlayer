// MediaPlayer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

//#include "RingBuffer.h"
//#include <thread>

#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>
//#include "opencv2/opencv.hpp"


#include "FFmpegDecoder.h"
#include "SDLPlayer.h"
#include "RTMPReceiver.h"


int main(int argc, char **argv)
{
	//**1**//

	RTMPReceiver receiver;

					
 	//char url[] = "rtmp://47.97.169.87:1935/openstream/test2";
	//rtmp://120.77.203.179:1935/openstream/test3
 	//receiver.RTMPReceiverInit(url);
 	char url[] = "rtmp://47.97.169.87:1935/openstream/test2";
	//char url[] = "rtmp://120.77.203.179:1935/openstream/test2";
 	receiver.RTMPReceiverInit(url,false,0);
	
	bool i=receiver.RTMPReceiverStart();
	if (!i)
	{
		std::cout << "error";
	}
	std::cin.get();
	return 0;
}

//using namespace std;
//#ifdef WIN32  
//#include <windows.h>
//#endif
//
////opencv_world340d.lib
//#pragma comment(lib, "opencv_world340d.lib")
////#pragma comment(lib, "opencv_core245d.lib")
////#pragma comment(lib, "opencv_highgui245d.lib")
//
//
//
//#pragma comment(lib, "librtmp.lib")
//#ifdef WIN32
//#pragma comment(lib,"WS2_32.lib")
//#pragma comment(lib,"winmm.lib")
//#endif

//planeBegin//
#define HTON16(x)  ((x>>8&0xff)|(x<<8&0xff00))
#define HTON24(x)  ((x>>16&0xff)|(x<<16&0xff0000)|x&0xff00)
#define HTON32(x)  ((x>>24&0xff)|(x>>8&0xff00)|\
	(x << 8 & 0xff0000) | (x << 24 & 0xff000000))

#define STR(x) (x.c_str())
#define FCUR(x) (ftell(x))
#define FSEEK(x,f) (fseek(f,x,SEEK_CUR))
#define FSET(x,f) (fseek(f,x,SEEK_SET))
//#define LOG(x,f) (fprintf(f,STR(x)))


FILE *flvfile = NULL;
FILE *h264file = NULL;
unsigned int sizeFCUR;

char flvfilename[256] = "receive.flv";
char h264filename[256] = "test.h264";

/*strncpy(flvfilename, "test1.flv", sizeof(flvfilename));*/
/*strncpy(h264filename, "test1.h264", sizeof(h264filename));*/
bool hasname = false;
bool Init()
{
	//flvfile = fopen(flvfilename, "rb");
	fopen_s(&flvfile , flvfilename , "rb");

	//h264file = fopen(h264filename, "wb");
	fopen_s(&h264file, h264filename, "wb");
	
	if (flvfile == NULL || h264file == NULL)
	{
		return false;
	}
	fseek(flvfile, 0, SEEK_END);
	sizeFCUR = FCUR(flvfile);
	fseek(flvfile, 0, SEEK_SET);
	return true;
}
void Clear()
{
	fclose(flvfile);
	fclose(h264file);
}


bool Read8(int &i8, FILE*f)
{
	if (fread(&i8, 1, 1, f) != 1)
		return false;
	return true;
}
bool Read16(int &i16, FILE*f)
{
	if (fread(&i16, 2, 1, f) != 1)
		return false;
	i16 = HTON16(i16);
	return true;
}
bool Read24(int &i24, FILE*f)
{
	if (fread(&i24, 3, 1, f) != 1)
		return false;
	i24 = HTON24(i24);
	return true;
}
bool Read32(int &i32, FILE*f)
{
	if (fread(&i32, 4, 1, f) != 1)
		return false;
	i32 = HTON32(i32);
	return true;
}
bool Peek8(int &i8, FILE*f)
{
	if (fread(&i8, 1, 1, f) != 1)
		return false;
	fseek(f, -1, SEEK_CUR);
	return true;
}

bool ReadTime(int &itime, FILE*f)
{
	int temp = 0;
	if (fread(&temp, 4, 1, f) != 1)
		return false;
	itime = HTON24(temp);
	itime |= (temp & 0xff000000);
	return true;
}

bool ReadHead()
{
	int headlength = 0;
	int filetype = 0;
	if (!Read24(filetype, flvfile))
		return false;
	int typel = 'flv';
	int typeh = 'FLV';
	if (filetype != typeh && filetype != typel)
	{
		printf("not flv file\n");
		return false;
	}
	FSEEK(2, flvfile);
	if (!Read32(headlength, flvfile))
		return false;
	printf("headlength:%d\n", headlength);
	/////////跳过头部长度/////
	fseek(flvfile, 0, SEEK_SET);
	FSEEK(headlength, flvfile);
	return true;
}

//void H264JIEXI(int datelength);
void ReadBody()
{

	while (true)
	{
		FSEEK(4, flvfile);
		int type = 0;
		int time = 0;
		int htime = 0;
		int datelength = 0;
		int info = 0;

		char buff[256] = { 0 };
		if (!Read8(type, flvfile))
			break;
		if (!Read24(datelength, flvfile))
			break;
		if (!ReadTime(time, flvfile))
			break;
		////////跳过StreamID/////
		FSEEK(3, flvfile);
		if (!Peek8(info, flvfile))
			break;

		int pos = FCUR(flvfile);
		if (type == 9)
//			H264JIEXI(datelength);
		FSET(pos + datelength, flvfile);
	}
}

int h264space = 0x01000000;//H264内容间隔标识00000001

//void H264JIEXI(int datelength)//关键
//{
//	int info = 0;
//	Read8(info, flvfile);
//	int avctype = 0;
//	Read8(avctype, flvfile);
//	FSEEK(3, flvfile);
//	int templength = 0;
//	char*tempbuff = NULL;
//	if (avctype == 0)
//	{
//		FSEEK(6, flvfile);
//
//		Read16(templength, flvfile);
//		printf("sssize:%d\n", templength);
//
//		tempbuff = (char*)malloc(templength);
//		fread(tempbuff, 1, templength, flvfile);
//		fwrite(&h264space, 1, 4, h264file);
//		fwrite(tempbuff, 1, templength, h264file);
//		free(tempbuff);
//
//		Read8(templength, flvfile);//ppsnum
//
//		Read16(templength, flvfile);//ppssize
//		printf("ppsize:%d\n", templength);
//
//		tempbuff = (char*)malloc(templength);
//		fread(tempbuff, 1, templength, flvfile);
//		fwrite(&h264space, 1, 4, h264file);
//		fwrite(tempbuff, 1, templength, h264file);
//		free(tempbuff);
//	}
//	else
//	{
//		//	Read32(templength,flvfile);
//		//	tempbuff=(char*)malloc(templength);
//		//	fread(tempbuff,1,templength,flvfile);
//		//	fwrite(&h264space,1,4,h264file);
//		//	fwrite(tempbuff,1,templength,h264file);
//		//	free(tempbuff);
//		//可能存在多个nal，需全部读取
//		int countsize = 2 + 3;
//		while (countsize < datelength)
//		{
//			Read32(templength, flvfile);
//			tempbuff = (char*)malloc(templength);
//			fread(tempbuff, 1, templength, flvfile);
//			fwrite(&h264space, 1, 4, h264file);
//			fwrite(tempbuff, 1, templength, h264file);
//			free(tempbuff);
//			countsize += (templength + 4);
//		}
//	}
//}
//planeEnd//



//typedef struct video_info
//{
//	AVPacket *packet;
//	AVFrame *pAvFrame;
//	AVCodec         *pCodec;
//	AVFormatContext *pFormatCtx;
//	AVCodecContext  *pCodecCtx;
//	SwsContext *img_convert_ctx;
//	AVCodecParserContext *parser;
//	int videoindex;
//}video_t;
//
//video_t* video_init(const char* video_filename, int *ret)
//{
//	video_t* video_info = (video_t*)malloc(sizeof(video_t));
//	video_info->packet = NULL;
//	video_info->pAvFrame = NULL;
//	video_info->pCodec = NULL;
//	video_info->pFormatCtx = NULL;
//	video_info->pCodecCtx = NULL;
//	video_info->img_convert_ctx = NULL;
//	video_info->parser = NULL;
//	video_info->videoindex = -1;
//	av_register_all();
//
//	//文件流 输入流
//	if (avformat_open_input(&(video_info->pFormatCtx), video_filename, NULL, NULL) != 0)
//	{
//		//无法打开文件
//		(*ret) = -1;
//		return NULL;
//	}
//
//	//
//	if (avformat_find_stream_info(video_info->pFormatCtx, NULL) < 0)
//	{
//		//无法查找到流信息
//		(*ret) = -2;
//		return NULL;
//	}
//	av_dump_format(video_info->pFormatCtx, -1, video_filename, NULL);
//
//
//
//	for (int i = 0; i < video_info->pFormatCtx->nb_streams; i++)
//	{
//
//		if (video_info->pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
//		{
//			video_info->videoindex = i;
//			//break;
//		}
//	}
//	if (video_info->videoindex == -1)
//	{
//		//无法找到视频流
//		(*ret) = -3;
//		return NULL;
//	}
//
//
//
//
//	//video_info->pCodecCtx = avcodec_alloc_context3(video_info->pCodec);
//	video_info->pCodecCtx = avcodec_alloc_context3(NULL);
//	if (avcodec_parameters_to_context(video_info->pCodecCtx, video_info->pFormatCtx->streams[video_info->videoindex]->codecpar) < 0)
//	{
//		(*ret) = -4;
//		return NULL;
//	}
//
//	// 视频解码器查找
//// 	if (!(video_info->pCodec = avcodec_find_decoder(video_info->pFormatCtx->streams[video_info->videoindex]->codecpar->codec_id)))
//// 	{
//// 		printf("Can not find input video decoder! (没有找到合适的解码器！)\n");
//// 		return false;
//// 	}
//
//	video_info->pCodec = avcodec_find_decoder(video_info->pCodecCtx->codec_id);
//	if (!video_info->pCodec)
//	{
//		(*ret) = -5;
//		return NULL;
//	}
//
//// 	video_info->parser = av_parser_init(video_info->pCodec->id);
//// 	if (!video_info->parser) {
//// 		fprintf(stderr, "parser not found\n");
//// 		exit(1);
//// 	}
//	
//// 	video_info->pCodec = avcodec_find_decoder(video_info->pCodecCtx->codec_id);
//// 	if (video_info->pCodec == NULL)
//// 	{
//// 		(*ret) = -4;
//// 		return NULL;
//// 	}
//
//
//
// 	if (avcodec_open2(video_info->pCodecCtx, video_info->pCodec, NULL) < 0)
// 	{
// 		//无法打开解码器
// 		(*ret) = -5;
// 		return NULL;
// 	}
//	video_info->pAvFrame = av_frame_alloc();
//	int y_size = video_info->pCodecCtx->width * video_info->pCodecCtx->height;
//	video_info->packet = (AVPacket *)av_malloc(sizeof(AVPacket));
//	av_new_packet(video_info->packet, y_size);
//	(*ret) = 0;
//	return video_info;
//
//}
//
//void video_getimg(AVCodecContext * pCodecCtx, SwsContext * img_convert_ctx, AVFrame * pFrame, cv::Mat* pCvMat)
//{
//	if (pCvMat->empty())
//	{
//		pCvMat->create(cv::Size(pCodecCtx->width, pCodecCtx->height), CV_8UC3);
//	}
//
//	AVFrame *pFrameRGB = NULL;
//	uint8_t  *out_bufferRGB = NULL;
//	pFrameRGB = av_frame_alloc();
//
//	//给pFrameRGB帧加上分配的内存;
//	int size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height,1);
//	
////	int size = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
//	out_bufferRGB = new uint8_t[size];
////	avpicture_fill((AVPicture *)pFrameRGB, out_bufferRGB, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
//
//	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_bufferRGB, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);
//
//	//YUV to RGB
//	sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);
//
//	memcpy(pCvMat->data, out_bufferRGB, size);
//
//	delete[] out_bufferRGB;
//	av_free(pFrameRGB);
//}
//
//int video_get_alltime(video_t* handel)
//{
//	int hours, mins, secs, us;
//	if (handel->pFormatCtx->duration != AV_NOPTS_VALUE)
//	{
//		int64_t duration = handel->pFormatCtx->duration + 5000;
//		secs = duration / AV_TIME_BASE;
//		us = duration % AV_TIME_BASE;
//		mins = secs / 60;
//		secs %= 60;
//		hours = mins / 60;
//		mins %= 60;
//		return (hours * 3600 + mins * 60 + secs);
//	}
//	else
//	{
//		return 0;
//	}
//}
//
//int video_seek_frame(video_t* handel, long time_start)
//{
//	int64_t seek_pos = 0;
//	if (time_start < 0)
//	{
//		return -1;
//	}
//	seek_pos = time_start * AV_TIME_BASE;
//	if (handel->pFormatCtx->start_time != AV_NOPTS_VALUE)
//	{
//		seek_pos += handel->pFormatCtx->start_time;
//	}
//	if (av_seek_frame(handel->pFormatCtx, -1, seek_pos, AVSEEK_FLAG_ANY) < 0)
//	{
//		return -2;
//	}
//	return 0;
//}
//int video_get_frame(video_t* handel, cv::Mat* pCvMat)
//{
//	int result = 0;
//	int pic_got = -1;
//// 	result = av_read_frame(handel->pFormatCtx, handel->packet);
//// 	if (result < 0)
//// 	{
//// 		//视频播放完成
//// 		pCvMat = NULL;
//// 		return -6;
//// 	}
//
//	while (av_read_frame(handel->pFormatCtx, handel->packet)>=0)
//	{
//
//
//
//		//此处需注意，视频播放完成后，并不会输出-6，而是会再进行解码导致解码错误输出-7
//		if (handel->packet->stream_index == handel->videoindex)
//		{
//
//			int ret;
//			if ((ret = avcodec_send_packet(handel->pCodecCtx, handel->packet)) != 0)
//				return -1;
//
//
//			if ((ret = avcodec_receive_frame(handel->pCodecCtx, handel->pAvFrame)) != 0)
//				return -1;
//
//
//				if (handel->img_convert_ctx == NULL)
//				{
//					handel->img_convert_ctx = sws_getContext(handel->pCodecCtx->width, handel->pCodecCtx->height, handel->pCodecCtx->pix_fmt, handel->pCodecCtx->width, handel->pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
//				}
//				if (pCvMat->empty())
//				{
//					pCvMat->create(cv::Size(handel->pCodecCtx->width, handel->pCodecCtx->height), CV_8UC3);
//				}
//
//				if (handel->img_convert_ctx != NULL)
//				{
//					video_getimg(handel->pCodecCtx, handel->img_convert_ctx, handel->pAvFrame, pCvMat);
//				}
//			}	
//	}
//	//av_free_packet(handel->packet);
//	av_packet_unref(handel->packet);
//	return 0;
//}
//
//int video_uninit(video_t* handel)
//{
//	if (handel != NULL)
//	{
//		av_packet_unref(handel->packet);
//		avcodec_close(handel->pCodecCtx);
//		avformat_close_input(&(handel->pFormatCtx));
//		return 0;
//	}
//	else
//	{
//		return -1;
//	}
//}
//
//
//
//
//
//static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,const char *filename)
//{
//	char buf[1024];
//	int ret;
//
//	ret = avcodec_send_packet(dec_ctx, pkt);
//	if (ret < 0) {
//		fprintf(stderr, "Error sending a packet for decoding\n");
//		exit(1);
//	}
//
//	while (ret >= 0) {
//		ret = avcodec_receive_frame(dec_ctx, frame);
//		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
//			return;
//		else if (ret < 0) {
//			fprintf(stderr, "Error during decoding\n");
//			exit(1);
//		}
//
//		printf("saving frame %3d\n", dec_ctx->frame_number);
//		fflush(stdout);
//
//		/* the picture is allocated by the decoder. no need to
//		free it */
//		snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
//		//pgm_save(frame->data[0], frame->linesize[0],frame->width, frame->height, buf);
//	}
//}



////Refresh
//#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)
//
//int thread_exit = 0;
////Thread
//int sfp_refresh_thread(void *opaque)
//{
//	//SDL_Event event;
//	//while (thread_exit == 0) {
//	//	event.type = SFM_REFRESH_EVENT;
//	//	SDL_PushEvent(&event);
//	//	//Wait 40 ms
//	//	SDL_Delay(40);
//	//}
//	return 0;
//}
//
////RingBuffer* pRingbuf = new RingBuffer();
////线程1：pRingbuf->Write();     //参数参考函数声明
////线程2: pRingbuf->Read();
//
//void planA()
//{
//	//**2**//
//
//
//
//	 	const char *filename, *outfilename;
//	 	const AVCodec *codec;
//	 	AVCodecParserContext *parser;
//	 	AVCodecContext *c = NULL;
//	 	FILE *f;
//	 	AVFrame *frame;
//	 	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
//	 	uint8_t *data;
//	 	size_t   data_size;
//	 	int ret;
//	 	AVPacket *pkt;
//	 
//	 	//if (argc <= 2) {
//	 	//	fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
//	 	//	exit(0);
//	 	//}
//	 	filename ="cuc_ieschool.h264";
//	 	outfilename = "out.yuv";
//	 
//	 	pkt = av_packet_alloc();
//	 	if (!pkt)
//	 		exit(1);
//	 
//	 	/* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
//	 	memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
//	 
//	 	/* find the MPEG-1 video decoder */
//	 	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
//	 	if (!codec) {
//	 		fprintf(stderr, "Codec not found\n");
//	 		exit(1);
//	 	}
//	 
//	 	parser = av_parser_init(codec->id);
//	 	if (!parser) {
//	 		fprintf(stderr, "parser not found\n");
//	 		exit(1);
//	 	}
//	 
//	 	c = avcodec_alloc_context3(codec);
//	 	if (!c) {
//	 		fprintf(stderr, "Could not allocate video codec context\n");
//	 		exit(1);
//	 	}
//	 
//	 	/* For some codecs, such as msmpeg4 and mpeg4, width and height
//	 	MUST be initialized there because this information is not
//	 	available in the bitstream. */
//	 
//	 	/* open it */
//	 	if (avcodec_open2(c, codec, NULL) < 0) {
//	 		fprintf(stderr, "Could not open codec\n");
//	 		exit(1);
//	 	}
//	 
//	 	//f = fopen(filename, "rb");
//	 	fopen_s(&f,filename, "rb");
//	 	if (!f) {
//	 		fprintf(stderr, "Could not open %s\n", filename);
//	 		exit(1);
//	 	}
//	 
//	 	frame = av_frame_alloc();
//	 	if (!frame) {
//	 		fprintf(stderr, "Could not allocate video frame\n");
//	 		exit(1);
//	 	}
//	 
//	 	while (!feof(f)) {
//	 		/* read raw data from the input file */
//	 		data_size = fread(inbuf, 1, INBUF_SIZE, f);
//	 		if (!data_size)
//	 			break;
//	 
//	 		/* use the parser to split the data into frames */
//	 		data = inbuf;
//	 		while (data_size > 0) {
//	 			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
//	 				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
//	 			if (ret < 0) {
//	 				fprintf(stderr, "Error while parsing\n");
//	 				exit(1);
//	 			}
//	 			data += ret;
//	 			data_size -= ret;
//	 
//	 			if (pkt->size)
//	 				decode(c, frame, pkt, outfilename);
//	 		}
//	 	}
//	 
//	 	/* flush the decoder */
//	 	decode(c, frame, NULL, outfilename);
//	 
//	 	fclose(f);
//	 
//	 	av_parser_close(parser);
//	 	avcodec_free_context(&c);
//	 	av_frame_free(&frame);
//	 	av_packet_free(&pkt);
//	// 
//	// 	return 0;
//	//**2**//
//}
//void planB()
//{
//	//****************************************************************
//
//	int ret = -1;
//	cv::Mat img;
//	int count = 0;
//	cv::Mat* pCvMat = new cv::Mat(); //解码输出数据
//	video_t* handel = video_init("cuc_ieschool.h264", &ret); //解码初始化
//	assert(handel != NULL);
//	assert(ret == 0);
//	//解码时间和视频定位仅支持视频文件类型，流媒体方式不支持
//	int alltime = video_get_alltime(handel); //获取解码时间
//	//DEBUG_PRINT("%d", alltime);
//	int state = video_seek_frame(handel, 0); //视频定位
//	//DEBUG_PRINT("%d", state);
//
//	while (1)
//	{
//		//if (count % JUMP_RATE == 0)
//		//{
//			int num = video_get_frame(handel, pCvMat); //获取解码数据
//			if (!(*pCvMat).empty())
//			{
//				(*pCvMat).copyTo(img);
//				resize(img, img, cv::Size(640, 480));
//				imshow("Test", img);
//				cvWaitKey(1);
//				count = 0;
//			}
//			if (num < 0)
//			{
//				//break;
//			}
//		//}
//		count++;
//	}
//	//释放资源
//	pCvMat->release();
//	video_uninit(handel);
//	
//	//****************************
//}
//int planC()
//{
//
//
//	//const char *fname = "123.h264";
//	const char *fname="receiveM.h264";
//	//const char *fname = "receiveH.h264";
//	//const char *fname = "receiveB.h264";
//	//const char *fname = "cuc_ieschool.h264";
//	//receive.h264
//	//const char *fname = "test.h264";
//	//const char *fname = "rtmp://47.97.169.87:1935/openstream/test2";
//
//	char errbuf[256] = { 0 };
//	int iRes = 0;
//	int vindex = -1;
//	AVFormatContext *fctx = NULL;
//	AVCodecContext *cctx = NULL;
//	AVCodec *c = NULL;
//	AVPacket *pkt = NULL;
//	AVFrame *fr = NULL;
//	AVFrame *yuv = NULL;
//	uint8_t *buf = NULL;
//	int vsize;
//	struct SwsContext *imgCtx = NULL;
//
//	SDL_Window *sw = NULL;
//	SDL_Renderer *sr = NULL;
//	SDL_Texture *ste = NULL;
//	SDL_Rect srect = { 0 };
//
//	av_register_all();  //ffmpeg 4.0 After no
//	if (SDL_Init(SDL_INIT_VIDEO) != 0)
//	{
//		cout << "SDL init failed!" << endl;
//		return -1;
//	}
//
//
//	fctx = avformat_alloc_context();
//	avformat_network_init();
//	if ((iRes = avformat_open_input(&fctx, fname, NULL, NULL)) != 0)
//	{
//		cout << "File open failed!" << endl;
//		return -1;
//	}
//
//	if (avformat_find_stream_info(fctx, NULL) < 0)
//	{
//		cout << "Stream find failed!\n";
//		return -1;
//	}
//	av_dump_format(fctx, -1, fname, NULL);
//
//	for (int i = 0; i < fctx->nb_streams; i++)
//	{
//		if (fctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
//			vindex = i;
//	}
//	if (vindex == -1)
//	{
//		cout << "Codec find failed!" << endl;
//		return -1;
//	}
//
//	cctx = avcodec_alloc_context3(NULL);
//	if (avcodec_parameters_to_context(cctx, fctx->streams[vindex]->codecpar) < 0)
//	{
//		cout << "Copy stream failed!" << endl;
//		return -1;
//	}
//	c = avcodec_find_decoder(cctx->codec_id);
//	if (!c) {
//		cout << "Find Decoder failed!" << endl;
//		return -1;
//	}
//	//cctx->time_base.den = 25;
//	if (avcodec_open2(cctx, c, NULL) != 0) {
//		cout << "Open codec failed!" << endl;
//		return -1;
//	}
//
//	sw = SDL_CreateWindow("video", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 680, 540, SDL_WINDOW_OPENGL);
//	sr = SDL_CreateRenderer(sw, -1, 0);
//	ste = SDL_CreateTexture(sr, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, cctx->width, cctx->height);
//	if (!sw || !sr || !ste) {
//		cout << "Create SDL windows failed!" << endl;
//		return -1;
//	}
//	srect.w = cctx->width;
//	srect.h = cctx->height;
//
//	imgCtx = sws_getContext(cctx->width, cctx->height, cctx->pix_fmt, cctx->width, cctx->height, AV_PIX_FMT_YUV420P,
//		SWS_BICUBIC, NULL, NULL, NULL);
//	if (!imgCtx) {
//		cout << "Get swscale context failed!" << endl;
//		return -1;
//	}
//	pkt = av_packet_alloc();
//	fr = av_frame_alloc();
//	yuv = av_frame_alloc();
//	vsize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cctx->width, cctx->height, 1);
//	buf = (uint8_t *)av_malloc(vsize);
//	av_image_fill_arrays(yuv->data, yuv->linesize, buf, AV_PIX_FMT_YUV420P, cctx->width, cctx->height, 1);
//
//	//while (true)
//	//{
//
//	unsigned int i = 0;
////	int m = cctx->time_base.den / cctx->time_base.num;
//	int m = 25;
//	int v = m * 60;
//		while (av_read_frame(fctx, pkt) >= 0) 
//		{
//			if (pkt->stream_index == vindex) 
//			{
//				//if (i%v==0)
//				//{
//					if ((iRes = avcodec_send_packet(cctx, pkt)) != 0)
//					{
//						cout << "Send video stream packet failed!" << endl;
//						av_strerror(iRes, errbuf, 256);
//						return -5;
//					}
//					if ((iRes = avcodec_receive_frame(cctx, fr)) != 0)
//					{
//						cout << "Receive video frame failed!\n";
//						av_strerror(iRes, errbuf, 256);
//						return -6;
//					}
//					sws_scale(imgCtx, fr->data, fr->linesize, 0, cctx->height, yuv->data, yuv->linesize);
//
//					SDL_UpdateTexture(ste, &srect, yuv->data[0], yuv->linesize[0]);
//					SDL_RenderClear(sr);
//					SDL_RenderCopy(sr, ste, NULL, NULL);
//					SDL_RenderPresent(sr);
//					//延时40ms
//					SDL_Delay(1000/(cctx->time_base.den / cctx->time_base.num));
//				//}
//			}
//			i++;
//		}
//	//}
//
//
//	av_free(buf);
//	av_frame_free(&yuv);
//	av_frame_free(&fr);
//	av_packet_free(&pkt);
//	sws_freeContext(imgCtx);
//	SDL_DestroyTexture(ste);
//	SDL_DestroyRenderer(sr);
//	SDL_DestroyWindow(sw);
//	SDL_Quit();
//	avcodec_free_context(&cctx);
//	avformat_close_input(&fctx);
//	avformat_free_context(fctx);
//
//}
//
//
//int InitSockets()
//{
//#ifdef WIN32
//	WORD version;
//	WSADATA wsaData;
//	version = MAKEWORD(1, 1);
//	return (WSAStartup(version, &wsaData) == 0);
//#endif
//}
//
//void CleanupSockets()
//{
//#ifdef WIN32
//	WSACleanup();
//#endif
//}
//
//
//
//int planD()
//{
//
//	InitSockets();
//
//	double duration = -1;
//	int nRead;
//	//is live stream ?
//	bool bLiveStream = true;
//
//
//	int bufsize = 1024 * 1024 * 10;
//	char *buf = (char*)malloc(bufsize);
//	memset(buf, 0, bufsize);
//	long countbufsize = 0;
//
//	//FILE *fp = fopen("receive.flv", "wb");
//
//	FILE *fp;
//	fopen_s(&fp,"receive.flv", "wb");
//	//fopen_s(&fp, "receive.h264", "wb");
//	if (!fp) {
//		RTMP_LogPrintf("Open File Error.\n");
//		CleanupSockets();
//		return -1;
//	}
//
//
//
//	RTMP *rtmp = RTMP_Alloc();
//	RTMP_Init(rtmp);
//	//set connection timeout,default 30s
//	rtmp->Link.timeout = 10;
//	// HKS's live URL
//	//if(!RTMP_SetupURL(rtmp,"rtmp://live.hkstv.hk.lxdns.com/live/hks"))
//	char  url[]= "rtmp://47.97.169.87:1935/openstream/test2";
//	if (!RTMP_SetupURL(rtmp, url))
//	{
//		RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		return -1;
//	}
//	if (bLiveStream) {
//		rtmp->Link.lFlags |= RTMP_LF_LIVE;
//	}
//
//	//1hour
//	RTMP_SetBufferMS(rtmp, 3600 * 1000);
//
//	if (!RTMP_Connect(rtmp, NULL)) {
//		RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		return -1;
//	}
//
//	if (!RTMP_ConnectStream(rtmp, 0)) {
//		RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
//		RTMP_Close(rtmp);
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		return -1;
//	}
//
//	//while (nRead = RTMP_Read(rtmp, buf, bufsize)) {
//	//	//RTMP_
//	//	fwrite(buf, 1, nRead, fp);
//	//	countbufsize += nRead;
//	//	RTMP_LogPrintf("Receive: %5dByte, Total: %5.2fkB\n", nRead, countbufsize*1.0 / 1024);
//	//}
//
//	//plan1//
//	RTMPPacket pc = { 0 };// ps = { 0 };
//	int jjj = 0;
//
//	unsigned char result = 0;
//	unsigned char nAVCPacketType = 0;
//	char *data = NULL;
//	static unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
//	
//	while (RTMP_ReadPacket(rtmp, &pc))
//	{
//		if (RTMPPacket_IsReady(&pc))	//是否读取完毕。((a)->m_nBytesRead == (a)->m_nBodySize)  
//		{
//			if (!pc.m_nBodySize)
//				continue;
//			if (pc.m_packetType == RTMP_PACKET_TYPE_VIDEO && RTMP_ClientPacket(rtmp, &pc))
//			{
//				result = pc.m_body[0];
//				nAVCPacketType = pc.m_body[1];
//				data = pc.m_body;
//				bool bIsKeyFrame = false;
//				if (result == 0x17)//I frame
//				{
//					bIsKeyFrame = true;
//				}
//				else if (result == 0x27)
//				{
//					bIsKeyFrame = false;
//				}
//				if (nAVCPacketType == 0)
//				{
//					//AVCsequence header
//					//Access to SPS
//					int spsnum = data[10] & 0x1f;
//					int number_sps = 11;
//					int count_sps = 1;
//					while (count_sps <= spsnum) {
//						int spslen = (data[number_sps] & 0x000000FF) << 8 | (data[number_sps + 1] & 0x000000FF);
//						number_sps += 2;
//						fwrite(start_code, 1, 4, fp);
//						fwrite(data + number_sps, 1, spslen, fp);
//						number_sps += spslen;
//						count_sps++;
//					}
//					//Get PPS
//					int ppsnum = data[number_sps] & 0x1f;
//					int number_pps = number_sps + 1;
//					int count_pps = 1;
//					while (count_pps <= ppsnum) {
//						int ppslen = (data[number_pps] & 0x000000FF) << 8 | data[number_pps + 1] & 0x000000FF;
//						number_pps += 2;
//						fwrite(start_code, 1, 4, fp);
//						fwrite(data + number_pps, 1, ppslen, fp);
//						number_pps += ppslen;
//						count_pps++;
//					}
//				}
//				else if (nAVCPacketType == 1)
//				{
//					//AVC NALU
//					int len = 0;
//					int num = 5;
//					while (num < pc.m_nBodySize)
//					{
//						len = (data[num] & 0x000000FF) << 24 | (data[num + 1] & 0x000000FF) << 16 | (data[num + 2] & 0x000000FF) << 8 | data[num + 3] & 0x000000FF;
//						num = num + 4;
//						fwrite(data + num, 1, len, fp);
//						fwrite(start_code, 1, 4, fp);
//						num = num + len;
//					}
//				}
//				else if (nAVCPacketType == 2)
//				{
//					//AVC end of sequence (lower level NALU sequence ender is not required or supported)
//				}
//			}
//			else if (pc.m_packetType == RTMP_PACKET_TYPE_INFO && RTMP_ClientPacket(rtmp, &pc))
//			{
//				int nVideoCodecId = 0;
//				int nVideoWidth = 255;
//				int nVideoHeight = 188;
//				int nVideoFrameRate = 25;
//				int nAudioCodecId = 0;
//				int nAudioSampleRate = 0;
//				int nAudioSampleSize = 0;
//				bool bStereo = false;			//立体声
//				int nFileSize = 0;
//				//ParseScriptTag(pc.m_body, pc.m_nBodySize, nVideoCodecId, nVideoWidth, nVideoHeight, nVideoFrameRate, nAudioCodecId, nAudioSampleRate, nAudioSampleSize, bStereo, nFileSize);
//				int k = 0;
//			}
//			RTMPPacket_Free(&pc);
//		}
//	}
//	//plan1//
//	if (fp)
//		fclose(fp);
//
//	if (buf) {
//		free(buf);
//	}
//
//	if (rtmp) {
//		RTMP_Close(rtmp);
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		rtmp = NULL;
//	}
//
//	return 0;
//}
//
//
//
//
//static int Decode(unsigned char *inputbuf, size_t size)
//{
//	
//}
//int planE()
//{
//	//strncpy(flvfilename, "test1.flv", sizeof(flvfilename));
//	//strncpy(h264filename, "test1.h264", sizeof(h264filename));
//	hasname = true;
//
//	if (!Init())
//	{
//		cout << "Init Err" << endl;
//		return -1;
//	}
//	if (!ReadHead())
//	{
//		cout << "ReadHead Err" << endl;
//		return -1;
//	}
//	ReadBody();
//	cout << "read over" << endl;
//	Clear();
//	return 0;
//}
//
//
////解析ScriptTag
//void ParseScriptTag(char *pBuffer, int nBufferLength, int &nVideoCodecId, int &nVideoWidth, int &nVideoHeight, int &nFrameRate, int &nAudioCodecId, int &nAudioSampleRate, int &nAudioSampleSize, bool &bStereo, int &nFileSize)
//{
//	AMFObject obj;
//	AVal val;
//	AMFObjectProperty * property;
//	AMFObject subObject;
//	int nRes = AMF_Decode(&obj, pBuffer, nBufferLength, FALSE);
//	if (nRes < 0)
//	{
//		//RTMP_Log(RTMP_LOGERROR, "%s, error decoding invoke packet", __FUNCTION__);
//		return;
//	}
//
//	AMF_Dump(&obj);
//	for (int n = 0; n < obj.o_num; n++)
//	{
//		property = AMF_GetProp(&obj, NULL, n);
//		if (property != NULL)
//		{
//			if (property->p_type == AMF_OBJECT)
//			{
//				AMFProp_GetObject(property, &subObject);
//				for (int m = 0; m < subObject.o_num; m++)
//				{
//					property = AMF_GetProp(&subObject, NULL, m);
//					if (property != NULL)
//					{
//						if (property->p_type == AMF_OBJECT)
//						{
//
//						}
//						else if (property->p_type == AMF_BOOLEAN)
//						{
//							int bVal = AMFProp_GetBoolean(property);
//							if (strncmp("stereo", property->p_name.av_val, property->p_name.av_len) == 0)
//							{
//								bStereo = bVal > 0 ? true : false;
//							}
//						}
//						else if (property->p_type == AMF_NUMBER)
//						{
//							double dVal = AMFProp_GetNumber(property);
//							if (strncmp("width", property->p_name.av_val, property->p_name.av_len) == 0)
//							{
//								nVideoWidth = (int)dVal;
//							}
//							else if (_stricmp("height", property->p_name.av_val) == 0)
//							{
//								nVideoHeight = (int)dVal;
//							}
//							else if (_stricmp("framerate", property->p_name.av_val) == 0)
//							{
//								nFrameRate = (int)dVal;
//							}
//							else if (_stricmp("videocodecid", property->p_name.av_val) == 0)
//							{
//								nVideoCodecId = (int)dVal;
//							}
//							else if (_stricmp("audiosamplerate", property->p_name.av_val) == 0)
//							{
//								nAudioSampleRate = (int)dVal;
//							}
//							else if (_stricmp("audiosamplesize", property->p_name.av_val) == 0)
//							{
//								nAudioSampleSize = (int)dVal;
//							}
//							else if (_stricmp("audiocodecid", property->p_name.av_val) == 0)
//							{
//								nAudioCodecId = (int)dVal;
//							}
//							else if (_stricmp("filesize", property->p_name.av_val) == 0)
//							{
//								nFileSize = (int)dVal;
//							}
//						}
//						else if (property->p_type == AMF_STRING)
//						{
//							AMFProp_GetString(property, &val);
//						}
//
//					}
//				}
//			}
//			else
//			{
//				AMFProp_GetString(property, &val);
//			}
//		}
//	}
//}
//int planF()
//{
//	FFmpegDecoder * decoder=new FFmpegDecoder();
//	decoder->DecoderInit();
//	//ffmpeg//
//
//	InitSockets();
//	bool bLiveStream = true;
//	//FILE *fp = fopen("receive.h264", "wb");
//	FILE *fp;
//	fopen_s(&fp, "receiveM.h264", "wb");
//
//
//	if (!fp) {
//		RTMP_LogPrintf("Open File Error.\n");
//		CleanupSockets();
//		return -1;
//	}
//	RTMP *rtmp = RTMP_Alloc();
//	RTMP_Init(rtmp);
//	//set connection timeout,default 30s
//	rtmp->Link.timeout = 30;
//	// HKS's live URL
//	//if(!RTMP_SetupURL(rtmp,"rtmp://live.hkstv.hk.lxdns.com/live/hks"))
//	//if (!RTMP_SetupURL(rtmp, "rtmp://120.77.203.179:1935/openstream/test"))
//	char url[] = "rtmp://47.97.169.87:1935/openstream/test2";
//	//char url[] = "rtmp://live.hkstv.hk.lxdns.com/live/hks";
//	//char url[] = "rtmp://120.77.203.179:1935/openstream/test";
//	if (!RTMP_SetupURL(rtmp,url ))
//	{
//		RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		return -1;
//	}
//	if (bLiveStream) {
//		rtmp->Link.lFlags |= RTMP_LF_LIVE;
//	}
//
//	//1hour
//	//RTMP_SetBufferMS(rtmp, 3600 * 1000);
//	//10 minute
//	RTMP_SetBufferMS(rtmp, 600 * 1000);
//
//	if (!RTMP_Connect(rtmp, NULL)) {
//		RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		return -1;
//	}
//
//	if (!RTMP_ConnectStream(rtmp, 0)) {
//		RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
//		RTMP_Close(rtmp);
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		return -1;
//	}
//
//	RTMPPacket pc = { 0 };// ps = { 0 };
//	unsigned int nFrameType = 0;
//	unsigned char result = 0;
//	unsigned char nAVCPacketType = 0;
//	int nRetClientPacket = 0;
//	char *data = NULL;
//	int jjj = 0;
//	static unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
//	bool bIsFirst = true;
//	bool bStartWrite = false;
//	bool bIsInit = false;
//	uint8_t *cur_ptr;
//	int cur_size;
//	while (RTMP_ReadPacket(rtmp, &pc))
//	{
//		if (RTMPPacket_IsReady(&pc))	//是否读取完毕。((a)->m_nBytesRead == (a)->m_nBodySize)  
//		{
//			if (!pc.m_nBodySize)
//				continue;
//			if (pc.m_packetType == RTMP_PACKET_TYPE_VIDEO && RTMP_ClientPacket(rtmp, &pc))
//			{
//
//				unsigned char framedata[1024 * 100];	
//				int m_currentPos = 0;
//
//				result = pc.m_body[0];
//				nAVCPacketType = pc.m_body[1];
//				data = pc.m_body;
//				bool bIsKeyFrame = false;
//				if (result == 0x17)//I frame
//				{
//					bIsKeyFrame = true;
//				}
//				else if (result == 0x27)
//				{
//					bIsKeyFrame = false;
//				}
//				if (bIsKeyFrame&&bIsFirst)
//				{
//					bStartWrite = true;
//					bIsFirst = false;
//				}
//
//				if (bStartWrite)
//				{
//
//
//					if (nAVCPacketType == 0)//AVCSequence Header
//					{
//						//unsigned char framedata[1024 * 100];// [pc.m_nBodySize + 1]
//						////unsigned char * framedata = nullptr;// [1024 * 100];// [pc.m_nBodySize + 1]
//						//int m_currentPos = 0;
//						//AVCsequence header
//						//Access to SPS
//						int spsnum = data[10] & 0x1f;
//						int number_sps = 11;
//						int count_sps = 1;
//						while (count_sps <= spsnum) 
//						{
//							int spslen = (data[number_sps] & 0x000000FF) << 8 | (data[number_sps + 1] & 0x000000FF);
//							number_sps += 2;
//							fwrite(start_code, 1, 4, fp);
//							
//							memcpy(framedata+m_currentPos, start_code, 4);
//							m_currentPos +=4;
//
//							fwrite(data + number_sps, 1, spslen, fp);
//
//							memcpy(framedata + m_currentPos, (data + number_sps), spslen);
//							
//
//							number_sps += spslen;
//
//							m_currentPos += spslen;
//
//							count_sps++;
//						}
//						//decode////////////////////////////////////////
//						
//
//						cur_size = m_currentPos;
//						cur_ptr = framedata;
//						decoder->DecodeOnePacket(cur_size, cur_ptr);
//
//						//decode////////////////////////////////////////////////////////////////
//						//Get PPS
//						int ppsnum = data[number_sps] & 0x1f;
//						int number_pps = number_sps + 1;
//						int count_pps = 1;
//						while (count_pps <= ppsnum)
//						{
//							int ppslen = (data[number_pps] & 0x000000FF) << 8 | data[number_pps + 1] & 0x000000FF;
//							number_pps += 2;
//							fwrite(start_code, 1, 4, fp);
//
//							memcpy(framedata+ m_currentPos, start_code, 4);
//							m_currentPos += 4;
//
//							fwrite(data + number_pps, 1, ppslen, fp);
//							memcpy(framedata + m_currentPos, (data + number_pps), ppslen);
//							
//
//							number_pps += ppslen;
//							
//							m_currentPos += ppslen;
//
//							count_pps++;
//						}
//						//decode////////////////////////////////////////
//
//						cur_size = m_currentPos;
//						cur_ptr = framedata;
//						decoder->DecodeOnePacket(cur_size, cur_ptr);
//
//						//decode////////////////////////////////////////////////////////////////
//					}
//					else if (nAVCPacketType == 1)//AVC NALU
//					{
//
//						int len = 0;
//						int outlen = 0;
//						int num = 5;
//						
//						while (num < pc.m_nBodySize)
//						{
//							len = (data[num] & 0x000000FF) << 24 | (data[num + 1] & 0x000000FF) << 16 | (data[num + 2] & 0x000000FF) << 8 | data[num + 3] & 0x000000FF;
//							num = num + 4;
//
//							fwrite(start_code, 1, 4, fp);
//							memcpy(framedata + m_currentPos, start_code, 4);
//							m_currentPos += 4;
//
//							fwrite(data + num, 1, len, fp);
//							memcpy(framedata + m_currentPos, (data + num), len);
//							num = num + len;
//							m_currentPos += len;
//						}
//						//decode////////////////////////////////////////
//
//						cur_size = m_currentPos;
//						cur_ptr = framedata;
//						decoder->DecodeOnePacket(cur_size, cur_ptr);
//						
//						//decode////////////////////////////////////////////////////////////////
//						
//					}
//					else if (nAVCPacketType == 2)
//					{
//						//AVC end of sequence (lower level NALU sequence ender is not required or supported)
//					}
//				}
//
//			}
//			else if (pc.m_packetType == RTMP_PACKET_TYPE_INFO && RTMP_ClientPacket(rtmp, &pc))
//			{
//				int nVideoCodecId = 0;
//				int nVideoWidth = 1280;
//				int nVideoHeight = 720;
//				int nVideoFrameRate = 25;
//				int nAudioCodecId = 0;
//				int nAudioSampleRate = 0;
//				int nAudioSampleSize = 0;
//				bool bStereo = false;			//立体声
//				int nFileSize = 0;
//				ParseScriptTag(pc.m_body, pc.m_nBodySize, nVideoCodecId, nVideoWidth, nVideoHeight, nVideoFrameRate, nAudioCodecId, nAudioSampleRate, nAudioSampleSize, bStereo, nFileSize);
//				int k = 0;
//			}
//			RTMPPacket_Free(&pc);
//		}
//	}
//
//	if (fp)
//		fclose(fp);
//
//	if (rtmp) {
//		RTMP_Close(rtmp);
//		RTMP_Free(rtmp);
//		CleanupSockets();
//		rtmp = NULL;
//	}
//}





