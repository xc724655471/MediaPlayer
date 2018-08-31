#pragma once
#include "librtmp/rtmp_sys.h"
#include "librtmp/log.h"

//#pragma comment(lib, "librtmp.lib")
#ifdef WIN32
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"winmm.lib")
#endif

class RTMPReceiver
{
public:
	RTMPReceiver();
	~RTMPReceiver();
public:
	bool RTMPReceiverInit(char * url,bool bWriteFile=false,int streamtype = 1);
	bool RTMPReceiverStart();
	int InitSockets();
	void CleanupSockets();
	void ParseScriptTag(char *pBuffer, int nBufferLength, int &nVideoCodecId, int &nVideoWidth, int &nVideoHeight, int &nFrameRate, int &nAudioCodecId, int &nAudioSampleRate, int &nAudioSampleSize, bool &bStereo, int &nFileSize);
public:
	bool bLiveStream;
	//FILE *fp = fopen("receive.h264", "wb");
	FILE *fp;
	RTMPPacket pc ;// ps = { 0 };
	unsigned int nFrameType ;
	unsigned char result ;
	unsigned char nAVCPacketType ;
	int nRetClientPacket ;
	char *data ;
	int jjj ;
	//static unsigned char const start_code[4];
	bool bIsFirst ;
	bool bStartWrite ;
	bool bIsInit ;
	uint8_t *cur_ptr;
	int cur_size;
	RTMP *m_pRtmp ;
	bool m_IsWriteFile;
	int StreamType;
};

