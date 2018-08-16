#include "stdafx.h"
#include "RingBuffer.h"

#define MAX_RING_BUFFER_SIZE (2*1024*1000)
#define MIN_RING_BUFFER_SIZE (128*1000)
#define DEFAULT_RING_BUFFER_SIZE (512*1000)

RingBuffer::RingBuffer()
{
	m_iWidx = 0;
	m_iRIdx = 0;
	m_uBufferSize = DEFAULT_RING_BUFFER_SIZE;
	m_pBuffer = new char[m_uBufferSize];
	m_bOutsideBuf = false;
	m_iWorkStatus = RINGBUFFER_NORMAL;
	//InitializeCriticalSectionAndSpinCount(&m_csBufMutex, 4000);
}

RingBuffer::RingBuffer(unsigned bufferSize, char* pBuffer)
{
	m_iWidx = 0;
	m_iRIdx = 0;
// 	if (bufferSize < MIN_RING_BUFFER_SIZE)
// 	{
// 		bufferSize = MIN_RING_BUFFER_SIZE;
// 	}
// 	if (bufferSize > MAX_RING_BUFFER_SIZE)
// 	{
// 		bufferSize = MAX_RING_BUFFER_SIZE;
// 	}

	//outside buf do not check size
	m_uBufferSize = bufferSize;
	m_pBuffer = pBuffer;
	m_bOutsideBuf = true;
	m_iWorkStatus = RINGBUFFER_NORMAL;
}

RingBuffer::RingBuffer(unsigned bufferSize)
{
	m_iWidx = 0;
	m_iRIdx = 0;
	if (bufferSize < MIN_RING_BUFFER_SIZE)
	{
		bufferSize = MIN_RING_BUFFER_SIZE;
	}
	if (bufferSize > MAX_RING_BUFFER_SIZE)
	{
		bufferSize = MAX_RING_BUFFER_SIZE;
	}
	m_uBufferSize = bufferSize;
	m_pBuffer = new char[m_uBufferSize];
	m_bOutsideBuf = false;
	m_iWorkStatus = RINGBUFFER_NORMAL;
}


RingBuffer::~RingBuffer()
{
	if (!m_bOutsideBuf && m_pBuffer)
	{
		delete[]m_pBuffer;
		m_pBuffer = NULL;
	}
//	DeleteCriticalSection(&m_csBufMutex);

}

//left 1Byte for guard
int RingBuffer::GetFreeBufferBytes(int iRidx, int iWidx)
{
	if (iRidx == iWidx)
	{
		return m_uBufferSize-1;
	}
	else if (iWidx > iRidx)
	{
		return (iRidx - iWidx + m_uBufferSize - 1);
	}
	else
	{
		return (iRidx - iWidx - 1);
	}
}

//this func can  write to the addr = ridx-1 (at most)
int RingBuffer::Write(const unsigned char* pBuf, unsigned writeLen)
{
	//m_pBuffer may alloc memory failed
	if (!m_pBuffer)
	{
		return -1;
	}

	int iRidx = m_iRIdx;
	int iWidx = m_iWidx;
	if (!pBuf || 0 == writeLen || GetFreeBufferBytes(iRidx, iWidx) < writeLen)
	{
		return -1;
	}

	int len1 = 0;
	if (m_iWidx < iRidx)
	{
		memcpy(&m_pBuffer[m_iWidx], pBuf, writeLen);
		m_iWidx += writeLen;
	}
	else
	{
		len1 = m_uBufferSize - m_iWidx;
		if (writeLen <= len1)
		{
			memcpy(&m_pBuffer[m_iWidx], pBuf, writeLen);
			m_iWidx += writeLen;
		}
		else
		{
			memcpy(&m_pBuffer[m_iWidx], pBuf, len1);
			memcpy(m_pBuffer, pBuf + len1, writeLen - len1);
			m_iWidx = writeLen - len1;
		}
	}

	return writeLen;
}

void RingBuffer::Init(){}
//
int RingBuffer::Read(unsigned char* pBuf, unsigned readLen)
{
	//m_pBuffer may alloc memory failed
	if (!m_pBuffer)
	{
		return -1;
	}

	if (!pBuf)
	{
		return -1;
	}

	int iWidx = m_iWidx;
	int iRidx = m_iRIdx;
	int bufferDataLen = m_uBufferSize - GetFreeBufferBytes(iRidx, iWidx) - 1;
	if (bufferDataLen <= readLen)
	{
		//can not use readall here, because GetFreeBufferBytes func and readall func may use the different
		//ridx and widx
		return ReadToWidx(pBuf, iWidx);
	}
	else  
	{
		if (m_iRIdx < iWidx)
		{
			memcpy(pBuf, &m_pBuffer[m_iRIdx], readLen);
			m_iRIdx += readLen;
		} 
		else
		{
			int len1 = m_uBufferSize - m_iRIdx;
			if (len1 >= readLen)
			{
				memcpy(pBuf, &m_pBuffer[m_iRIdx], readLen);
				m_iRIdx += readLen;
			}
			else
			{
				memcpy(pBuf, &m_pBuffer[m_iRIdx], len1);
				memcpy(pBuf + len1, m_pBuffer, readLen - len1);
				m_iRIdx = readLen - len1;
			}
		} //end m_iRIdx >= m_iWidx

		return readLen;
	}//end bufferDataLen > readLen
}

// read to widx
int RingBuffer::ReadToWidx(unsigned char* pBuf, int iWidx)
{
	//m_pBuffer may alloc memory failed
	if (!m_pBuffer)
	{
		return -1;
	}
	if (!pBuf || m_iWidx == m_iRIdx)
	{
		return -1;
	}

	int curWidx = m_iWidx;
	if (m_iRIdx < curWidx)
	{
		if (iWidx < m_iRIdx || iWidx > curWidx)
		{
			return -1;
		}
	}
	else
	{
		if (iWidx > curWidx && iWidx < m_iRIdx)
		{
			return -1;
		}
	}

	//must use temp varible here
	//int iWidx = m_iWidx;
	int readLen = 0;
	if (m_iRIdx > iWidx)
	{
		memcpy(pBuf, &m_pBuffer[m_iRIdx], m_uBufferSize - m_iRIdx);
		memcpy(pBuf + m_uBufferSize - m_iRIdx, m_pBuffer, iWidx);
		readLen = m_uBufferSize - m_iRIdx + iWidx;
	}
	else
	{
		memcpy(pBuf, &m_pBuffer[m_iRIdx], iWidx - m_iRIdx);
		readLen = iWidx - m_iRIdx;
	}
	//###can not set m_iRIdx = m_iWidx!!!!!
	m_iRIdx = iWidx;

	return readLen;
}

int RingBuffer::ReadAll(unsigned char* pBuf)
{
	//m_pBuffer may alloc memory failed
	if (!m_pBuffer)
	{
		return -1;
	}
	if (!pBuf || m_iWidx == m_iRIdx)
	{
		return -1;
	}

	return ReadToWidx(pBuf, m_iWidx);
}

char* RingBuffer::GetBufferAddr()
{
	return m_pBuffer;
}

int RingBuffer::GetBufferSize()
{
	return m_uBufferSize;
}

#if 0
//read to widx
int RingBuffer::ReadAll(unsigned char* pBuf)
{
	if (!pBuf || m_iWidx == m_iRIdx)
	{
		return -1;
	}
	
	//must use temp varible here
	int iWidx = m_iWidx;
	int readLen = 0;
	if (m_iRIdx > iWidx)
	{
		memcpy(pBuf, &m_pBuffer[m_iRIdx], m_uBufferSize - m_iRIdx);
		memcpy(pBuf + m_uBufferSize - m_iRIdx, m_pBuffer, iWidx);
		readLen = m_uBufferSize - m_iRIdx + iWidx;
	}
	else
	{
		memcpy(pBuf, &m_pBuffer[m_iRIdx], iWidx - m_iRIdx);
		readLen = iWidx - m_iRIdx;
	}
	//###can not set m_iRIdx = m_iWidx!!!!!
	m_iRIdx = iWidx;

	return readLen;
}
#endif

RINGBUFFER_WORK_STATUS RingBuffer::GetStatus()
{
	return m_iWorkStatus;
}

int RingBuffer::SetStatus(RINGBUFFER_WORK_STATUS status)
{
	if (status < RINGBUFFER_NORMAL || status >= RINGBUFFER_END)
	{
		return -1;
	}
	m_iWorkStatus = status;
	return 0;
}

int RingBuffer::Analyze()
{
	return 0;
}

void RingBuffer::Lock()
{
//	EnterCriticalSection(&m_csBufMutex);
}

void RingBuffer::UnLock()
{
//	LeaveCriticalSection(&m_csBufMutex);
}

void RingBuffer::Stop()
{
	SetStatus(RINGBUFFER_STOP);
}

int RingBuffer::GetFreeBufferBytes()
{
	return GetFreeBufferBytes(m_iRIdx, m_iWidx);
}

int RingBuffer::GetDataBytes()
{
	return m_uBufferSize - GetFreeBufferBytes();
}