#pragma once

#include <map>
#include <string>
#include <vector>

enum RINGBUFFER_WORK_STATUS
{
	RINGBUFFER_NORMAL,
	RINGBUFFER_CLOSEWAIT,
	RINGBUFFER_CLOSEACK,
	RINGBUFFER_STOP,
	RINGBUFFER_END         //guard status
};

class RingBuffer
{

public:
	RingBuffer();
	RingBuffer(unsigned bufferSize);
	//use pBufferOutside
	RingBuffer(unsigned bufferSize, char* pBuffer);
	virtual ~RingBuffer();
private:
	volatile int m_iWidx;
	volatile int m_iRIdx;
	char*        m_pBuffer;
	unsigned     m_uBufferSize;

	RINGBUFFER_WORK_STATUS m_iWorkStatus;
	//for bufpool
	bool m_bOutsideBuf;

	int ReadToWidx(unsigned char* pBuf, int iWidx);
	int GetFreeBufferBytes(int iRidx, int iWidx);

//	CRITICAL_SECTION m_csBufMutex;

public:
	virtual void Init();
	virtual int Read(unsigned char* pBuf, unsigned readLen);
	virtual int ReadAll(unsigned char* pBuf);
	virtual int Write(const unsigned char* pBuf, unsigned writeLen);
	virtual int Analyze();
	virtual void Stop();
	int GetFreeBufferBytes();
	int GetDataBytes();
	void SetRingBufferSize(unsigned bufferSize);
	char* GetBufferAddr();
	RINGBUFFER_WORK_STATUS GetStatus();
	int SetStatus(RINGBUFFER_WORK_STATUS status);
	void Lock();
	void UnLock();
	int GetBufferSize();

};