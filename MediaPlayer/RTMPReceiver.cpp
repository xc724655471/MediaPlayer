#include "stdafx.h"
#include "RTMPReceiver.h"
#include "FFmpegDecoder.h"

static unsigned char const start_code[4] = { 0x00, 0x00, 0x00, 0x01 };
RTMPReceiver::RTMPReceiver()
{
	bLiveStream = true;
	bool bLiveStream;
	//FILE *fp = fopen("receive.h264", "wb");
	//FILE *fp;
	 pc = { 0 };// ps = { 0 };
	  nFrameType = 0;
	  result = 0;
  nAVCPacketType = 0;
	 nRetClientPacket = 0;
	 data = NULL;
	 jjj = 0;
	
	 bIsFirst = true;
	 bStartWrite = false;
	 bIsInit = false;
	//uint8_t *cur_ptr;
	 cur_size=0;
	 rtmp = RTMP_Alloc();
	 m_IsWriteFile = false;
}


RTMPReceiver::~RTMPReceiver()
{
	if (m_IsWriteFile)
	{
		if (fp)
		fclose(fp);

	}

	if (rtmp)
	{
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		rtmp = NULL;
	}

}

bool RTMPReceiver::RTMPReceiverInit(char * url, bool bWriteFile)
{
	m_IsWriteFile = bWriteFile;
	InitSockets();


	if (m_IsWriteFile)
	{
		fopen_s(&fp, "receiveM.h264", "wb");
		if (!fp) {
			RTMP_LogPrintf("Open File Error.\n");
			CleanupSockets();
			return -1;
		}
	}
	//RTMP *rtmp = RTMP_Alloc();
	RTMP_Init(rtmp);

	//set connection timeout,default 30s
	rtmp->Link.timeout = 30;

	if (!RTMP_SetupURL(rtmp, url))
	{
		RTMP_Log(RTMP_LOGERROR, "SetupURL Err\n");
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}
	if (bLiveStream) {
		rtmp->Link.lFlags |= RTMP_LF_LIVE;
	}

	//1hour
	//RTMP_SetBufferMS(rtmp, 3600 * 1000);
	//10 minute
	RTMP_SetBufferMS(rtmp, 600 * 1000);

	if (!RTMP_Connect(rtmp, NULL)) {
		RTMP_Log(RTMP_LOGERROR, "Connect Err\n");
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}

	if (!RTMP_ConnectStream(rtmp, 0)) {
		RTMP_Log(RTMP_LOGERROR, "ConnectStream Err\n");
		RTMP_Close(rtmp);
		RTMP_Free(rtmp);
		CleanupSockets();
		return -1;
	}
}

bool RTMPReceiver::RTMPReceiverStart()
{
	FFmpegDecoder * decoder = new FFmpegDecoder();
	decoder->DecoderInit();

	while (RTMP_ReadPacket(rtmp, &pc))
	{
		if (RTMPPacket_IsReady(&pc))	//是否读取完毕。((a)->m_nBytesRead == (a)->m_nBodySize)  
		{
			if (!pc.m_nBodySize)
				continue;
			if (pc.m_packetType == RTMP_PACKET_TYPE_VIDEO && RTMP_ClientPacket(rtmp, &pc))
			{

				unsigned char framedata[1024 * 100];
				int m_currentPos = 0;

				result = pc.m_body[0];
				nAVCPacketType = pc.m_body[1];
				data = pc.m_body;
				bool bIsKeyFrame = false;
				if (result == 0x17)//I frame
				{
					bIsKeyFrame = true;
				}
				else if (result == 0x27)
				{
					bIsKeyFrame = false;
				}
				if (bIsKeyFrame&&bIsFirst)
				{
					bStartWrite = true;
					bIsFirst = false;
				}
				//提交用

				if (bStartWrite)
				{
					if (nAVCPacketType == 0)//AVCSequence Header
					{

						//AVCsequence header
						//Access to SPS
						int spsnum = data[10] & 0x1f;
						int number_sps = 11;
						int count_sps = 1;
						while (count_sps <= spsnum)
						{
							int spslen = (data[number_sps] & 0x000000FF) << 8 | (data[number_sps + 1] & 0x000000FF);
							number_sps += 2;
							//fwrite(start_code, 1, 4, fp);

							memcpy(framedata + m_currentPos, start_code, 4);
							m_currentPos += 4;

							//fwrite(data + number_sps, 1, spslen, fp);

							memcpy(framedata + m_currentPos, (data + number_sps), spslen);

							if (m_IsWriteFile)
							{
								fwrite(start_code, 1, 4, fp);
								fwrite(data + number_sps, 1, spslen, fp);
							}


							number_sps += spslen;

							m_currentPos += spslen;

							count_sps++;
						}


						//decode////////////////////////////////////////
						cur_size = m_currentPos;
						cur_ptr = framedata;
						decoder->DecodeOnePacket(cur_size, cur_ptr);
						//decode////////////////////////////////////////////////////////////////


						//Get PPS
						int ppsnum = data[number_sps] & 0x1f;
						int number_pps = number_sps + 1;
						int count_pps = 1;
						while (count_pps <= ppsnum)
						{
							int ppslen = (data[number_pps] & 0x000000FF) << 8 | data[number_pps + 1] & 0x000000FF;
							number_pps += 2;
							//fwrite(start_code, 1, 4, fp);

							memcpy(framedata + m_currentPos, start_code, 4);
							m_currentPos += 4;

							//fwrite(data + number_pps, 1, ppslen, fp);
							memcpy(framedata + m_currentPos, (data + number_pps), ppslen);


							if (m_IsWriteFile)
							{
								fwrite(start_code, 1, 4, fp);
								fwrite(data + number_pps, 1, ppslen, fp);
							}

							number_pps += ppslen;

							m_currentPos += ppslen;

							count_pps++;
						}
						//decode////////////////////////////////////////
						cur_size = m_currentPos;
						cur_ptr = framedata;
						decoder->DecodeOnePacket(cur_size, cur_ptr);
						//decode////////////////////////////////////////////////////////////////
					}
					else if (nAVCPacketType == 1)//AVC NALU
					{

						int len = 0;
						int outlen = 0;
						int num = 5;

						while (num < pc.m_nBodySize)
						{
							len = (data[num] & 0x000000FF) << 24 | (data[num + 1] & 0x000000FF) << 16 | (data[num + 2] & 0x000000FF) << 8 | data[num + 3] & 0x000000FF;
							num = num + 4;

							//fwrite(start_code, 1, 4, fp);
							memcpy(framedata + m_currentPos, start_code, 4);
							m_currentPos += 4;

							//fwrite(data + num, 1, len, fp);
							memcpy(framedata + m_currentPos, (data + num), len);

							if (m_IsWriteFile)
							{
								fwrite(start_code, 1, 4, fp);
								fwrite(data + num, 1, len, fp);
							}
							num = num + len;
							m_currentPos += len;
						}
						//decode////////////////////////////////////////

						cur_size = m_currentPos;
						cur_ptr = framedata;
						decoder->DecodeOnePacket(cur_size, cur_ptr);

						//decode////////////////////////////////////////////////////////////////

					}
					else if (nAVCPacketType == 2)
					{
						//AVC end of sequence (lower level NALU sequence ender is not required or supported)
					}
				}

			}
			else if (pc.m_packetType == RTMP_PACKET_TYPE_INFO && RTMP_ClientPacket(rtmp, &pc))
			{
				int nVideoCodecId = 0;
				int nVideoWidth = 1280;
				int nVideoHeight = 720;
				int nVideoFrameRate = 25;
				int nAudioCodecId = 0;
				int nAudioSampleRate = 0;
				int nAudioSampleSize = 0;
				bool bStereo = false;			//立体声
				int nFileSize = 0;
				ParseScriptTag(pc.m_body, pc.m_nBodySize, nVideoCodecId, nVideoWidth, nVideoHeight, nVideoFrameRate, nAudioCodecId, nAudioSampleRate, nAudioSampleSize, bStereo, nFileSize);
				int k = 0;
			}
			RTMPPacket_Free(&pc);
		}
	}
	return false;
}

int RTMPReceiver::InitSockets()
{
#ifdef WIN32
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#endif
}

void RTMPReceiver::CleanupSockets()
{
#ifdef WIN32
	WSACleanup();
#endif
}

void RTMPReceiver::ParseScriptTag(char * pBuffer, int nBufferLength, int & nVideoCodecId, int & nVideoWidth, int & nVideoHeight, int & nFrameRate, int & nAudioCodecId, int & nAudioSampleRate, int & nAudioSampleSize, bool & bStereo, int & nFileSize)
{
	AMFObject obj;
	AVal val;
	AMFObjectProperty * property;
	AMFObject subObject;
	int nRes = AMF_Decode(&obj, pBuffer, nBufferLength, FALSE);
	if (nRes < 0)
	{
		//RTMP_Log(RTMP_LOGERROR, "%s, error decoding invoke packet", __FUNCTION__);
		return;
	}

	AMF_Dump(&obj);
	for (int n = 0; n < obj.o_num; n++)
	{
		property = AMF_GetProp(&obj, NULL, n);
		if (property != NULL)
		{
			if (property->p_type == AMF_OBJECT)
			{
				AMFProp_GetObject(property, &subObject);
				for (int m = 0; m < subObject.o_num; m++)
				{
					property = AMF_GetProp(&subObject, NULL, m);
					if (property != NULL)
					{
						if (property->p_type == AMF_OBJECT)
						{

						}
						else if (property->p_type == AMF_BOOLEAN)
						{
							int bVal = AMFProp_GetBoolean(property);
							if (strncmp("stereo", property->p_name.av_val, property->p_name.av_len) == 0)
							{
								bStereo = bVal > 0 ? true : false;
							}
						}
						else if (property->p_type == AMF_NUMBER)
						{
							double dVal = AMFProp_GetNumber(property);
							if (strncmp("width", property->p_name.av_val, property->p_name.av_len) == 0)
							{
								nVideoWidth = (int)dVal;
							}
							else if (_stricmp("height", property->p_name.av_val) == 0)
							{
								nVideoHeight = (int)dVal;
							}
							else if (_stricmp("framerate", property->p_name.av_val) == 0)
							{
								nFrameRate = (int)dVal;
							}
							else if (_stricmp("videocodecid", property->p_name.av_val) == 0)
							{
								nVideoCodecId = (int)dVal;
							}
							else if (_stricmp("audiosamplerate", property->p_name.av_val) == 0)
							{
								nAudioSampleRate = (int)dVal;
							}
							else if (_stricmp("audiosamplesize", property->p_name.av_val) == 0)
							{
								nAudioSampleSize = (int)dVal;
							}
							else if (_stricmp("audiocodecid", property->p_name.av_val) == 0)
							{
								nAudioCodecId = (int)dVal;
							}
							else if (_stricmp("filesize", property->p_name.av_val) == 0)
							{
								nFileSize = (int)dVal;
							}
						}
						else if (property->p_type == AMF_STRING)
						{
							AMFProp_GetString(property, &val);
						}

					}
				}
			}
			else
			{
				AMFProp_GetString(property, &val);
			}
		}
	}
}
