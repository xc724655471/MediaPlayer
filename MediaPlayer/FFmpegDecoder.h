#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include "libavutil/imgutils.h"

	//#include "SDL.h"
	//imgutils.c
#ifdef __cplusplus
}
#endif

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")	
#pragma comment(lib ,"swscale.lib")




class FFmpegDecoder
{
public:
	FFmpegDecoder();
	~FFmpegDecoder();
public:
	const AVCodec *codec;
	AVCodecContext *cctx = nullptr;
	AVCodecParserContext *pCodecParserCtx = NULL;
	int frame_count;
	AVFrame *frame;
	AVPacket  avpkt;
	//AVPacket  *avpkt;
	AVFrame *yuv = NULL;

	struct SwsContext *imgCtx = NULL;
	bool bIsInit;
	int vsize;
public:
	bool DecoderInit(int streamtype);
	bool DecodeOnePacket(int cur_size, uint8_t *cur_ptr);
	int StreamType;

};

