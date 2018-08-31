#include "stdafx.h"
#include "FFmpegDecoder.h"
#include "SDLPlayer.h"



SDLPlayer * player = new SDLPlayer;
FFmpegDecoder::FFmpegDecoder()
{
	//const AVCodec *codec;
	cctx = nullptr;
	pCodecParserCtx = NULL;
	frame_count=0;
	//AVFrame *frame;
	//AVPacket  avpkt;
	AVPacket  *avpkt;
	yuv = NULL;

	imgCtx = NULL;
	//bool bIsInit;
	bIsInit = false;
	vsize = 0;
	StreamType = 1;
}


FFmpegDecoder::~FFmpegDecoder()
{
}

bool FFmpegDecoder::DecoderInit(int streamtype)
{
	StreamType = streamtype;

	avcodec_register_all();

	av_init_packet(&avpkt);
	if (StreamType)
	{
		codec = avcodec_find_decoder(AV_CODEC_ID_H265);
	} 
	else
	{
		codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	}

	
	if (!codec) {
		fprintf(stderr, "Codec not found\n");
		exit(1);
	}
	cctx = avcodec_alloc_context3(codec);
	if (!cctx) {
		fprintf(stderr, "Could not allocate video codec context\n");
		exit(1);
	}

	if (StreamType)
	{
		pCodecParserCtx = av_parser_init(AV_CODEC_ID_H265); //初始化 AVCodecParserContext
	} 
	else
	{
		pCodecParserCtx = av_parser_init(AV_CODEC_ID_H264); //初始化 AVCodecParserContext
	}
	
	if (!pCodecParserCtx) {
		printf("Could not allocate video parser context\n");
		return -1;
	}

	if (codec->capabilities&CODEC_CAP_TRUNCATED)
		cctx->flags |= CODEC_FLAG_TRUNCATED;	/* we do not send complete frames */

	if (avcodec_open2(cctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
		exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		exit(1);
	}


	return true;
}

bool FFmpegDecoder::DecodeOnePacket(int cur_size, uint8_t *cur_ptr)
{

	while (cur_size > 0)
	{

		unsigned char *buf = 0;
		int buf_len = 0;

		// 解析获得一个Packet
 		int len = av_parser_parse2(
 			pCodecParserCtx, cctx,
 			&avpkt.data, &avpkt.size,
 			cur_ptr, cur_size,
 			NULL, NULL, NULL);

		cur_ptr += len;
		cur_size -= len;

		if (avpkt.size)
		{
			//if (avcodec_send_packet(cctx, avpkt))
			if (avcodec_send_packet(cctx, &avpkt))
			{
				printf("%s %d avcodec_send_packet fail\n", __func__, __LINE__);
				return -1;
			}
			if (avcodec_receive_frame(cctx, frame))
			{
				printf("%s %d avcodec_receive_frame fail\n", __func__, __LINE__);
				return -1;
			}
			if (!bIsInit)
			{


				player->SDLPlayerInit(cctx->width, cctx->height);

				imgCtx = sws_getContext(cctx->width, cctx->height, cctx->pix_fmt, cctx->width, cctx->height, AV_PIX_FMT_YUV420P,
					SWS_BICUBIC, NULL, NULL, NULL);

				if (!imgCtx)
				{
					//cout << "Get swscale context failed!" << endl;
					return -1;
				}

				yuv = av_frame_alloc();
				vsize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, cctx->width, cctx->height, 1);
				buf = (uint8_t *)av_malloc(vsize);
				av_image_fill_arrays(yuv->data, yuv->linesize, buf, AV_PIX_FMT_YUV420P, cctx->width, cctx->height, 1);
				bIsInit = true;
			}

			if (bIsInit)
			{
				sws_scale(imgCtx, frame->data, frame->linesize, 0, cctx->height, yuv->data, yuv->linesize);

				//float time = cctx->time_base.den / cctx->time_base.num;
				//float time = cctx->time_base.den / cctx->time_base.num;
				player->Play(yuv->data[0], yuv->linesize[0], 40);
			}

			
		}
	}
	return true;
}
