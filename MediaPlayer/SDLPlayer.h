#pragma once

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




class AVFrame;


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
	//int sfp_refresh_thread(void *opaque);
};

