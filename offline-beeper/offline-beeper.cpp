#include "offline-beeper.h"


OBS_DECLARE_MODULE()


bool _stopping = false;

std::mutex audioMutex;
std::thread beepThread;

Mix_Chunk* disconnectedBeepSound;


const char* obs_module_author(void)
{
	return "DefiantZombie";
}


const char* obs_module_name(void)
{
	return "Stream Offline Beeper";
}


const char* obs_module_description(void)
{
	return "Plays a sound file if the stream drops.";
}


void PlaySound(Mix_Chunk* sound)
{
	int result;

	audioMutex.lock();

	result = Mix_PlayChannel(-1, sound, 0);
	if (result == -1)
	{
		warning("Error playing. (result: %d), %s", result, Mix_GetError());

		audioMutex.unlock();
		return;
	}

	while (Mix_Playing(-1))
		SDL_Delay(1);

	audioMutex.unlock();
}


void DisconnectedBeep(void)
{
	PlaySound(disconnectedBeepSound);
}


void OnActivate(void* data, calldata_t* params)
{
	warning("%s", "received");
}


void OnStarting(void* data, calldata_t* params)
{
	warning("%s", "received");
}


void OnStart(void* data, calldata_t* params)
{
	warning("%s", "received");
}


void OnReconnect(void* data, calldata_t* params)
{
	int timeout = (int)calldata_int(params, "timeout_sec");
	warning("%s, %i", "received", timeout);
}


void OnReconnectSuccess(void* data, calldata_t* params)
{
	warning("%s", "received");
}


void OnDeactivate(void* data, calldata_t* params)
{
	warning("%s", "received");
	if (!_stopping)
	{
		if (beepThread.joinable())
		{
			beepThread.join();
		}
		beepThread = std::thread(DisconnectedBeep);
	}
}


void OnStopping(void* data, calldata_t* params)
{
	warning("%s", "received");
	_stopping = true;
}


void OnStop(void* data, calldata_t* params)
{
	int reason = (int)calldata_int(params, "code");
	warning("%s, %i", "received", reason);
}


void OnFrontendEvents(enum obs_frontend_event event, void *private_data)
{
	obs_output_t* output = obs_frontend_get_streaming_output();
	signal_handler_t* handler = nullptr;

	if (output)
	{
		handler = obs_output_get_signal_handler(output);
	}

	if (event == OBS_FRONTEND_EVENT_STREAMING_STARTED)
	{
		if (output)
		{

			signal_handler_connect(handler, "activate", OnActivate, 0);
			signal_handler_connect(handler, "starting", OnStarting, 0);
			signal_handler_connect(handler, "reconnect", OnReconnect, 0);
			signal_handler_connect(handler, "reconnect_success", OnReconnectSuccess, 0);
			signal_handler_connect(handler, "deactivate", OnDeactivate, 0); //
			signal_handler_connect(handler, "stopping", OnStopping, 0); //
			signal_handler_connect(handler,	"stop", OnStop, 0);
		}
	}
	else if (event == OBS_FRONTEND_EVENT_STREAMING_STOPPED)
	{
		if (output)
		{
			signal_handler_disconnect(handler, "activate", OnActivate, 0);
			signal_handler_disconnect(handler, "starting", OnStarting, 0);
			signal_handler_disconnect(handler, "reconnect", OnReconnect, 0);
			signal_handler_disconnect(handler, "reconnect_success", OnReconnectSuccess, 0);
			signal_handler_disconnect(handler, "deactivate", OnDeactivate, 0); //
			signal_handler_disconnect(handler, "stopping", OnStopping, 0); //
			signal_handler_disconnect(handler, "stop", OnStop, 0);
		}

		_stopping = false;
	}
}


bool InitSound(void)
{
	int result = 0;
	int flags = MIX_INIT_MP3;

	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		warning("Failed to init SDL, %s", SDL_GetError());
		return false;
	}

	if (flags != (result = Mix_Init(flags)))
	{
		warning("Could not initialize mixer (result: %d).", result);
		warning("Mix_Init: %s", Mix_GetError());
		return false;
	}

	if (Mix_OpenAudio(22050, AUDIO_S16SYS, 2, 1000) == -1)
	{
		warning("Failed to open mix audio, %s", Mix_GetError());
		return false;
	}

	std::string audio_path = obs_get_module_data_path(obs_current_module());
	//audio_path.append("/disconnected.mp3");
	bool fileFound = false;

	disconnectedBeepSound = Mix_LoadWAV(audio_path.append("/disconnected.mp3").c_str());
	if (disconnectedBeepSound) fileFound = true;
	
	if (!fileFound)
	{
		disconnectedBeepSound = Mix_LoadWAV(audio_path.append("/disconnected.wav").c_str());
		if (disconnectedBeepSound) fileFound = true;
	}

	if (!fileFound)
	{
		warning("Failed to load disconnected sound file.");
		return false;
	}

	return true;
}


bool obs_module_load(void)
{
	if (!InitSound())
	{
		warning("Sound initialization failed.");
		return false;
	}
	
	obs_frontend_add_event_callback(OnFrontendEvents, 0);

	return true;
}


void obs_module_unload(void)
{
	if (beepThread.joinable())
	{
		beepThread.join();
	}

	if (disconnectedBeepSound)
		Mix_FreeChunk(disconnectedBeepSound);

	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();
}
