#include "utils.c"

static SDL_AudioStream* stream = NULL;

// Sounds =============
static const char DUMMY_SOUND[] = {
#embed "../res/error.wav"
};

static SDL_AudioSpec dummy_sound_spec = {0};

static uint8_t* dummy_sound_data     = NULL;
static uint32_t dummy_sound_data_len = 0;
// ====================

int initSound(void) {
	// Init sound subsystem

	if(!SDL_Init(SDL_INIT_AUDIO)) {
		SDL_Die("Failed to launch audio subsystem");
	}

	if(!(stream = SDL_OpenAudioDeviceStream(
			 SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &dummy_sound_spec, NULL, NULL
		 ))) {
		SDL_Die("Failed to open audio device stream");
	}

	// Init sounds

	SDL_IOStream* io = SDL_IOFromConstMem(DUMMY_SOUND, sizeof(DUMMY_SOUND));
	if(!io) SDL_Die("Failed to load memory stream");

	if(!SDL_LoadWAV_IO(
		   io, true, &dummy_sound_spec, &dummy_sound_data, &dummy_sound_data_len
	   )) {
		SDL_Die("Failed to load dummy sound");
	}

	return 0;
}
