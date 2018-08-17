#include "stdafx.h"
#include "SDLPlayer.h"
#

SDLPlayer::SDLPlayer()
{
	 sw = NULL;
	 sr = NULL;
	 ste = NULL;
	 srect = { 0 };
	 buf = NULL;
}


SDLPlayer::~SDLPlayer()
{
}

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit = 0;
int thread_pause = 0;
int sfp_refresh_thread(void *opaque)
{

	thread_exit = 0;
	thread_pause = 0;

	while (thread_exit == 0) {
		if (!thread_pause) {
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	//Quit
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);
	thread_exit = 0;
	thread_pause = 0;
	return 0;
}

bool SDLPlayer::SDLPlayerInit(int width,int height)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		//cout << "SDL init failed!" << endl;
		return false;
	}

	sw = SDL_CreateWindow("video", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);

	sr = SDL_CreateRenderer(sw, -1, 0);

	ste = SDL_CreateTexture(sr, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (!sw || !sr || !ste)
	{
		//cout << "Create SDL windows failed!" << endl;
		return false;
	}
	srect.x = 0;
	srect.y = 0;
	srect.w = width;
	srect.h = height;

	video_tid = SDL_CreateThread(sfp_refresh_thread, NULL, NULL);

	return true;
}

void SDLPlayer::Play(uint8_t *yuvdate, int yuvlinesize, float time)
{


	SDL_WaitEvent(&event);
	if (event.type == SFM_REFRESH_EVENT)
	{
		//SDL_UpdateTexture(ste, &srect, yuv->data[0], yuv->linesize[0]);
		SDL_UpdateTexture(ste, &srect, yuvdate, yuvlinesize);
		SDL_RenderClear(sr);
		SDL_RenderCopy(sr, ste, NULL, NULL);
		SDL_RenderPresent(sr);
		//—” ±40ms
		SDL_Delay(1000 / time);
	}
	else if (event.type == SDL_QUIT) 
	{
		thread_exit = 1;
	}
	else if (event.type == SFM_BREAK_EVENT)
	{
		//break;
	}
}


