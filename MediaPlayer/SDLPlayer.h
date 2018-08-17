#pragma once

#include <queue>
#include <mutex>

#ifdef __cplusplus
extern "C" {
#endif



#include "SDL.h"
//#include "SDL2/SDL"
	//imgutils.c
#ifdef __cplusplus
}
#endif

#pragma comment(lib ,"SDL2.lib")
#pragma comment(lib, "SDL2main.lib")

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







class SDLPlayer
{
public:
	SDLPlayer();
	~SDLPlayer();
public:

	SDL_Window * sw  ;
	SDL_Renderer *sr  ;
	SDL_Texture *ste  ;
	SDL_Rect srect ;
	uint8_t *buf  ;
	SDL_Thread *video_tid;
	SDL_Event event;
	
public:
	bool SDLPlayerInit(int width, int height);
	void Play(uint8_t *yuvdate, int yuvlinesize, float time);
	void Play(PlayPacket * pakcet);
	//int sfp_refresh_thread(void *opaque);

	PlayPacket * m_pakcet;
	std::queue<PlayPacket *> playpack;
	std::mutex dataCacheMutex;       //通信数据缓存区的锁
	std::condition_variable cond_;   //线程的条件变量
	bool bisPlay;

	void ReceiveDataFromOtherThread(PlayPacket * packet);
	void run();//在类里定义一个死循环，用来执行此类的功能函数（方法）
};

