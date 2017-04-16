#pragma once

#include "obs.h"
#include "obs-module.h"
#include "obs-frontend-api/obs-frontend-api.h"
#include "signal.h"
#include <thread>
#include <sstream>
#include <mutex>

extern "C"
{
#include "SDL.h"
#include "SDL_mixer.h"
}


#define warning(format, ...) blog(LOG_WARNING, "[%s] %s -- " format, \
	obs_module_name(), \
	 __FUNCTION__, \
	##__VA_ARGS__)


const char* obs_module_author(void);
const char* obs_module_name(void);
const char* obs_module_description(void);

bool obs_module_load(void);
void obs_module_unload(void);

void OnActivate(void*, calldata_t*);
void OnStarting(void*, calldata_t*);
void OnStart(void*, calldata_t*);
void OnReconnect(void*, calldata_t*);
void OnReconnectSuccess(void*, calldata_t*);
void OnDeactivate(void*, calldata_t*);
void OnStopping(void*, calldata_t*);
void OnStop(void*, calldata_t*);

void OnFrontendEvents(enum obs_frontend_event, void*);

bool InitSound(void);
void PlayMusic(Mix_Music*);
void PlaySound(Mix_Chunk*);

void DisconnectedBeep(void);
