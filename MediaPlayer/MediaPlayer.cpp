// MediaPlayer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <stdio.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/opencv.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include "libavutil/imgutils.h"

#include "SDL.h"
	//imgutils.c
#ifdef __cplusplus
}
#endif
#define INBUF_SIZE 4096
//跳帧显示
#define JUMP_RATE 10

#define  _CRT_SECURE_NO_WARNINGS

using namespace std;


//opencv_world340d.lib
#pragma comment(lib, "opencv_world340d.lib")
//#pragma comment(lib, "opencv_core245d.lib")
//#pragma comment(lib, "opencv_highgui245d.lib")

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")	
#pragma comment(lib ,"swscale.lib")
#pragma comment(lib ,"SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

typedef struct video_info
{
	AVPacket *packet;
	AVFrame *pAvFrame;
	AVCodec         *pCodec;
	AVFormatContext *pFormatCtx;
	AVCodecContext  *pCodecCtx;
	SwsContext *img_convert_ctx;
	AVCodecParserContext *parser;
	int videoindex;
}video_t;

video_t* video_init(const char* video_filename, int *ret)
{
	video_t* video_info = (video_t*)malloc(sizeof(video_t));
	video_info->packet = NULL;
	video_info->pAvFrame = NULL;
	video_info->pCodec = NULL;
	video_info->pFormatCtx = NULL;
	video_info->pCodecCtx = NULL;
	video_info->img_convert_ctx = NULL;
	video_info->parser = NULL;
	video_info->videoindex = -1;
	av_register_all();

	//文件流 输入流
	if (avformat_open_input(&(video_info->pFormatCtx), video_filename, NULL, NULL) != 0)
	{
		//无法打开文件
		(*ret) = -1;
		return NULL;
	}

	//
	if (avformat_find_stream_info(video_info->pFormatCtx, NULL) < 0)
	{
		//无法查找到流信息
		(*ret) = -2;
		return NULL;
	}
	av_dump_format(video_info->pFormatCtx, -1, video_filename, NULL);



	for (int i = 0; i < video_info->pFormatCtx->nb_streams; i++)
	{

		if (video_info->pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			video_info->videoindex = i;
			//break;
		}
	}
	if (video_info->videoindex == -1)
	{
		//无法找到视频流
		(*ret) = -3;
		return NULL;
	}




	//video_info->pCodecCtx = avcodec_alloc_context3(video_info->pCodec);
	video_info->pCodecCtx = avcodec_alloc_context3(NULL);
	if (avcodec_parameters_to_context(video_info->pCodecCtx, video_info->pFormatCtx->streams[video_info->videoindex]->codecpar) < 0)
	{
		(*ret) = -4;
		return NULL;
	}

	// 视频解码器查找
// 	if (!(video_info->pCodec = avcodec_find_decoder(video_info->pFormatCtx->streams[video_info->videoindex]->codecpar->codec_id)))
// 	{
// 		printf("Can not find input video decoder! (没有找到合适的解码器！)\n");
// 		return false;
// 	}

	video_info->pCodec = avcodec_find_decoder(video_info->pCodecCtx->codec_id);
	if (!video_info->pCodec)
	{
		(*ret) = -5;
		return NULL;
	}

// 	video_info->parser = av_parser_init(video_info->pCodec->id);
// 	if (!video_info->parser) {
// 		fprintf(stderr, "parser not found\n");
// 		exit(1);
// 	}
	
// 	video_info->pCodec = avcodec_find_decoder(video_info->pCodecCtx->codec_id);
// 	if (video_info->pCodec == NULL)
// 	{
// 		(*ret) = -4;
// 		return NULL;
// 	}



 	if (avcodec_open2(video_info->pCodecCtx, video_info->pCodec, NULL) < 0)
 	{
 		//无法打开解码器
 		(*ret) = -5;
 		return NULL;
 	}
	video_info->pAvFrame = av_frame_alloc();
	int y_size = video_info->pCodecCtx->width * video_info->pCodecCtx->height;
	video_info->packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	av_new_packet(video_info->packet, y_size);
	(*ret) = 0;
	return video_info;

}

void video_getimg(AVCodecContext * pCodecCtx, SwsContext * img_convert_ctx, AVFrame * pFrame, cv::Mat* pCvMat)
{
	if (pCvMat->empty())
	{
		pCvMat->create(cv::Size(pCodecCtx->width, pCodecCtx->height), CV_8UC3);
	}

	AVFrame *pFrameRGB = NULL;
	uint8_t  *out_bufferRGB = NULL;
	pFrameRGB = av_frame_alloc();

	//给pFrameRGB帧加上分配的内存;
	int size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height,1);
	
//	int size = avpicture_get_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);
	out_bufferRGB = new uint8_t[size];
//	avpicture_fill((AVPicture *)pFrameRGB, out_bufferRGB, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height);

	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_bufferRGB, AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);

	//YUV to RGB
	sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

	memcpy(pCvMat->data, out_bufferRGB, size);

	delete[] out_bufferRGB;
	av_free(pFrameRGB);
}

int video_get_alltime(video_t* handel)
{
	int hours, mins, secs, us;
	if (handel->pFormatCtx->duration != AV_NOPTS_VALUE)
	{
		int64_t duration = handel->pFormatCtx->duration + 5000;
		secs = duration / AV_TIME_BASE;
		us = duration % AV_TIME_BASE;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
		return (hours * 3600 + mins * 60 + secs);
	}
	else
	{
		return 0;
	}
}

int video_seek_frame(video_t* handel, long time_start)
{
	int64_t seek_pos = 0;
	if (time_start < 0)
	{
		return -1;
	}
	seek_pos = time_start * AV_TIME_BASE;
	if (handel->pFormatCtx->start_time != AV_NOPTS_VALUE)
	{
		seek_pos += handel->pFormatCtx->start_time;
	}
	if (av_seek_frame(handel->pFormatCtx, -1, seek_pos, AVSEEK_FLAG_ANY) < 0)
	{
		return -2;
	}
	return 0;
}
int video_get_frame(video_t* handel, cv::Mat* pCvMat)
{
	int result = 0;
	int pic_got = -1;
// 	result = av_read_frame(handel->pFormatCtx, handel->packet);
// 	if (result < 0)
// 	{
// 		//视频播放完成
// 		pCvMat = NULL;
// 		return -6;
// 	}

	while (av_read_frame(handel->pFormatCtx, handel->packet)>=0)
	{



		//此处需注意，视频播放完成后，并不会输出-6，而是会再进行解码导致解码错误输出-7
		if (handel->packet->stream_index == handel->videoindex)
		{

			int ret;
			if ((ret = avcodec_send_packet(handel->pCodecCtx, handel->packet)) != 0)
				return -1;


			if ((ret = avcodec_receive_frame(handel->pCodecCtx, handel->pAvFrame)) != 0)
				return -1;


				if (handel->img_convert_ctx == NULL)
				{
					handel->img_convert_ctx = sws_getContext(handel->pCodecCtx->width, handel->pCodecCtx->height, handel->pCodecCtx->pix_fmt, handel->pCodecCtx->width, handel->pCodecCtx->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
				}
				if (pCvMat->empty())
				{
					pCvMat->create(cv::Size(handel->pCodecCtx->width, handel->pCodecCtx->height), CV_8UC3);
				}

				if (handel->img_convert_ctx != NULL)
				{
					video_getimg(handel->pCodecCtx, handel->img_convert_ctx, handel->pAvFrame, pCvMat);
				}
			}	
	}
	//av_free_packet(handel->packet);
	av_packet_unref(handel->packet);
	return 0;
}

int video_uninit(video_t* handel)
{
	if (handel != NULL)
	{
		av_packet_unref(handel->packet);
		avcodec_close(handel->pCodecCtx);
		avformat_close_input(&(handel->pFormatCtx));
		return 0;
	}
	else
	{
		return -1;
	}
}





static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,const char *filename)
{
	char buf[1024];
	int ret;

	ret = avcodec_send_packet(dec_ctx, pkt);
	if (ret < 0) {
		fprintf(stderr, "Error sending a packet for decoding\n");
		exit(1);
	}

	while (ret >= 0) {
		ret = avcodec_receive_frame(dec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during decoding\n");
			exit(1);
		}

		printf("saving frame %3d\n", dec_ctx->frame_number);
		fflush(stdout);

		/* the picture is allocated by the decoder. no need to
		free it */
		snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
		//pgm_save(frame->data[0], frame->linesize[0],frame->width, frame->height, buf);
	}
}



//Refresh
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

int thread_exit = 0;
//Thread
int sfp_refresh_thread(void *opaque)
{
	//SDL_Event event;
	//while (thread_exit == 0) {
	//	event.type = SFM_REFRESH_EVENT;
	//	SDL_PushEvent(&event);
	//	//Wait 40 ms
	//	SDL_Delay(40);
	//}
	return 0;
}


void planA()
{
	//**2**//



	 	const char *filename, *outfilename;
	 	const AVCodec *codec;
	 	AVCodecParserContext *parser;
	 	AVCodecContext *c = NULL;
	 	FILE *f;
	 	AVFrame *frame;
	 	uint8_t inbuf[INBUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];
	 	uint8_t *data;
	 	size_t   data_size;
	 	int ret;
	 	AVPacket *pkt;
	 
	 	//if (argc <= 2) {
	 	//	fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
	 	//	exit(0);
	 	//}
	 	filename ="cuc_ieschool.h264";
	 	outfilename = "out.yuv";
	 
	 	pkt = av_packet_alloc();
	 	if (!pkt)
	 		exit(1);
	 
	 	/* set end of buffer to 0 (this ensures that no overreading happens for damaged MPEG streams) */
	 	memset(inbuf + INBUF_SIZE, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	 
	 	/* find the MPEG-1 video decoder */
	 	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	 	if (!codec) {
	 		fprintf(stderr, "Codec not found\n");
	 		exit(1);
	 	}
	 
	 	parser = av_parser_init(codec->id);
	 	if (!parser) {
	 		fprintf(stderr, "parser not found\n");
	 		exit(1);
	 	}
	 
	 	c = avcodec_alloc_context3(codec);
	 	if (!c) {
	 		fprintf(stderr, "Could not allocate video codec context\n");
	 		exit(1);
	 	}
	 
	 	/* For some codecs, such as msmpeg4 and mpeg4, width and height
	 	MUST be initialized there because this information is not
	 	available in the bitstream. */
	 
	 	/* open it */
	 	if (avcodec_open2(c, codec, NULL) < 0) {
	 		fprintf(stderr, "Could not open codec\n");
	 		exit(1);
	 	}
	 
	 	//f = fopen(filename, "rb");
	 	fopen_s(&f,filename, "rb");
	 	if (!f) {
	 		fprintf(stderr, "Could not open %s\n", filename);
	 		exit(1);
	 	}
	 
	 	frame = av_frame_alloc();
	 	if (!frame) {
	 		fprintf(stderr, "Could not allocate video frame\n");
	 		exit(1);
	 	}
	 
	 	while (!feof(f)) {
	 		/* read raw data from the input file */
	 		data_size = fread(inbuf, 1, INBUF_SIZE, f);
	 		if (!data_size)
	 			break;
	 
	 		/* use the parser to split the data into frames */
	 		data = inbuf;
	 		while (data_size > 0) {
	 			ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
	 				data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
	 			if (ret < 0) {
	 				fprintf(stderr, "Error while parsing\n");
	 				exit(1);
	 			}
	 			data += ret;
	 			data_size -= ret;
	 
	 			if (pkt->size)
	 				decode(c, frame, pkt, outfilename);
	 		}
	 	}
	 
	 	/* flush the decoder */
	 	decode(c, frame, NULL, outfilename);
	 
	 	fclose(f);
	 
	 	av_parser_close(parser);
	 	avcodec_free_context(&c);
	 	av_frame_free(&frame);
	 	av_packet_free(&pkt);
	// 
	// 	return 0;
	//**2**//
}
void planB()
{
	//****************************************************************

	int ret = -1;
	cv::Mat img;
	int count = 0;
	cv::Mat* pCvMat = new cv::Mat(); //解码输出数据
	video_t* handel = video_init("cuc_ieschool.h264", &ret); //解码初始化
	assert(handel != NULL);
	assert(ret == 0);
	//解码时间和视频定位仅支持视频文件类型，流媒体方式不支持
	int alltime = video_get_alltime(handel); //获取解码时间
	//DEBUG_PRINT("%d", alltime);
	int state = video_seek_frame(handel, 0); //视频定位
	//DEBUG_PRINT("%d", state);

	while (1)
	{
		//if (count % JUMP_RATE == 0)
		//{
			int num = video_get_frame(handel, pCvMat); //获取解码数据
			if (!(*pCvMat).empty())
			{
				(*pCvMat).copyTo(img);
				resize(img, img, cv::Size(640, 480));
				imshow("Test", img);
				cvWaitKey(1);
				count = 0;
			}
			if (num < 0)
			{
				//break;
			}
		//}
		count++;
	}
	//释放资源
	pCvMat->release();
	video_uninit(handel);
	
	//****************************
}
int planC()
{
	const char *fname="E://cuc_ieschool.h264";

	char errbuf[256] = { 0 };
	int iRes = 0;
	int vindex = -1;
	AVFormatContext *fctx = NULL;
	AVCodecContext *cctx = NULL;
	AVCodec *c = NULL;
	AVPacket *pkt = NULL;
	AVFrame *fr = NULL;
	AVFrame *yuv = NULL;
	uint8_t *buf = NULL;
	int vsize;
	struct SwsContext *imgCtx = NULL;

	SDL_Window *sw = NULL;
	SDL_Renderer *sr = NULL;
	SDL_Texture *ste = NULL;
	SDL_Rect srect = { 0 };

	av_register_all();  //ffmpeg 4.0 After no
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		cout << "SDL init failed!" << endl;
		return -1;
	}


	fctx = avformat_alloc_context();
	if ((iRes = avformat_open_input(&fctx, fname, NULL, NULL)) != 0)
	{
		cout << "File open failed!" << endl;
		return -1;
	}

	if (avformat_find_stream_info(fctx, NULL) < 0)
	{
		cout << "Stream find failed!\n";
		return -1;
	}
	av_dump_format(fctx, -1, fname, NULL);

	for (int i = 0; i < fctx->nb_streams; i++)
	{
		if (fctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
			vindex = i;
	}
	if (vindex == -1)
	{
		cout << "Codec find failed!" << endl;
		return -1;
	}

	cctx = avcodec_alloc_context3(NULL);
	if (avcodec_parameters_to_context(cctx, fctx->streams[vindex]->codecpar) < 0)
	{
		cout << "Copy stream failed!" << endl;
		return -1;
	}
	c = avcodec_find_decoder(cctx->codec_id);
	if (!c) {
		cout << "Find Decoder failed!" << endl;
		return -1;
	}
	if (avcodec_open2(cctx, c, NULL) != 0) {
		cout << "Open codec failed!" << endl;
		return -1;
	}

	sw = SDL_CreateWindow("video", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 680, 540, SDL_WINDOW_OPENGL);
	sr = SDL_CreateRenderer(sw, -1, 0);
	ste = SDL_CreateTexture(sr, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, cctx->width, cctx->height);
	if (!sw || !sr || !ste) {
		cout << "Create SDL windows failed!" << endl;
		return -1;
	}
	srect.w = cctx->width;
	srect.h = cctx->height;

	imgCtx = sws_getContext(cctx->width, cctx->height, cctx->pix_fmt, cctx->width, cctx->height, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, NULL, NULL, NULL);
	if (!imgCtx) {
		cout << "Get swscale context failed!" << endl;
		return -1;
	}
	pkt = av_packet_alloc();
	fr = av_frame_alloc();
	yuv = av_frame_alloc();
	vsize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cctx->width, cctx->height, 1);
	buf = (uint8_t *)av_malloc(vsize);
	av_image_fill_arrays(yuv->data, yuv->linesize, buf, AV_PIX_FMT_YUV420P, cctx->width, cctx->height, 1);
	while (av_read_frame(fctx, pkt) >= 0) {
		if (pkt->stream_index == vindex) {
			if ((iRes = avcodec_send_packet(cctx, pkt)) != 0)
			{
				cout << "Send video stream packet failed!" << endl;
				av_strerror(iRes, errbuf, 256);
				return -5;
			}
			if ((iRes = avcodec_receive_frame(cctx, fr)) != 0)
			{
				cout << "Receive video frame failed!\n";
				av_strerror(iRes, errbuf, 256);
				return -6;
			}
			sws_scale(imgCtx, fr->data, fr->linesize, 0, cctx->height, yuv->data, yuv->linesize);

			SDL_UpdateTexture(ste, &srect, yuv->data[0], yuv->linesize[0]);
			SDL_RenderClear(sr);
			SDL_RenderCopy(sr, ste, NULL, NULL);
			SDL_RenderPresent(sr);
		}
	}

	av_free(buf);
	av_frame_free(&yuv);
	av_frame_free(&fr);
	av_packet_free(&pkt);
	sws_freeContext(imgCtx);
	SDL_DestroyTexture(ste);
	SDL_DestroyRenderer(sr);
	SDL_DestroyWindow(sw);
	SDL_Quit();
	avcodec_free_context(&cctx);
	avformat_close_input(&fctx);
	avformat_free_context(fctx);

}
int main(int argc, char **argv)
{
	//**1**//

	//**1**//
	//planB();
	planC();

	return 0;


	




}

