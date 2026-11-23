#include <stdio.h>
#include <stdlib.h>

#include "utils.c"

typedef struct {
	uint8_t* data;
	uint32_t data_len; // number of bytes in loaded file
	float    delay;    // in seconds

	size_t seek; // NOTE only used in copies
} Sound;

#define SOUNDS                                                                 \
	X(SOUND_BL, "bell.wav", 1.0)                                               \
	X(SOUND_CR_1, "carriage_return1.wav", 0.78)                                \
	X(SOUND_CR_2, "carriage_return2.wav", 0.86)                                \
	X(SOUND_CR_3, "carriage_return3.wav", 0.64)                                \
	X(SOUND_CR_4, "carriage_return4.wav", 0.69)                                \
	X(SOUND_HS_1, "hitstop1.wav", 0.18)                                        \
	X(SOUND_HS_2, "hitstop2.wav", 0.18)                                        \
	X(SOUND_HS_3, "hitstop3.wav", 0.20)                                        \
	X(SOUND_HS_4, "hitstop4.wav", 0.22)                                        \
	X(SOUND_LF_1, "linefeed1.wav", 0.24)                                       \
	X(SOUND_LF_2, "linefeed2.wav", 0.19)                                       \
	X(SOUND_LF_3, "linefeed3.wav", 0.17)                                       \
	X(SOUND_LF_4, "linefeed4.wav", 0.20)                                       \
	X(SOUND_WS_1, "space1.wav", 0.22)                                          \
	X(SOUND_WS_2, "space2.wav", 0.22)                                          \
	X(SOUND_WS_3, "space3.wav", 0.21)                                          \
	X(SOUND_WS_4, "space4.wav", 0.22)                                          \
	X(SOUND_TY_1, "typing1.wav", 0.06)                                         \
	X(SOUND_TY_2, "typing2.wav", 0.08)                                         \
	X(SOUND_TY_3, "typing3.wav", 0.09)                                         \
	X(SOUND_TY_4, "typing4.wav", 0.08)                                         \
	X(SOUND_TY_5, "typing5.wav", 0.09)

#define X(snd, path, dlay) extern Sound snd;

SOUNDS

#undef X

float playSound(Sound snd);

int initSound(void);

#if __INCLUDE_LEVEL__ == 0 ////////////////////

static uint8_t SOUND_BL_DAT[] = {
#embed "../res/bell.wav"
};
static uint8_t SOUND_CR_1_DAT[] = {
#embed "../res/carriage_return1.wav"
};
static uint8_t SOUND_CR_2_DAT[] = {
#embed "../res/carriage_return2.wav"
};
static uint8_t SOUND_CR_3_DAT[] = {
#embed "../res/carriage_return3.wav"
};
static uint8_t SOUND_CR_4_DAT[] = {
#embed "../res/carriage_return4.wav"
};
static uint8_t SOUND_HS_1_DAT[] = {
#embed "../res/hitstop1.wav"
};
static uint8_t SOUND_HS_2_DAT[] = {
#embed "../res/hitstop2.wav"
};
static uint8_t SOUND_HS_3_DAT[] = {
#embed "../res/hitstop3.wav"
};
static uint8_t SOUND_HS_4_DAT[] = {
#embed "../res/hitstop4.wav"
};
static uint8_t SOUND_LF_1_DAT[] = {
#embed "../res/linefeed1.wav"
};
static uint8_t SOUND_LF_2_DAT[] = {
#embed "../res/linefeed2.wav"
};
static uint8_t SOUND_LF_3_DAT[] = {
#embed "../res/linefeed3.wav"
};
static uint8_t SOUND_LF_4_DAT[] = {
#embed "../res/linefeed4.wav"
};
static uint8_t SOUND_WS_1_DAT[] = {
#embed "../res/space1.wav"
};
static uint8_t SOUND_WS_2_DAT[] = {
#embed "../res/space2.wav"
};
static uint8_t SOUND_WS_3_DAT[] = {
#embed "../res/space3.wav"
};
static uint8_t SOUND_WS_4_DAT[] = {
#embed "../res/space4.wav"
};
static uint8_t SOUND_TY_1_DAT[] = {
#embed "../res/typing1.wav"
};
static uint8_t SOUND_TY_2_DAT[] = {
#embed "../res/typing2.wav"
};
static uint8_t SOUND_TY_3_DAT[] = {
#embed "../res/typing3.wav"
};
static uint8_t SOUND_TY_4_DAT[] = {
#embed "../res/typing4.wav"
};
static uint8_t SOUND_TY_5_DAT[] = {
#embed "../res/typing5.wav"
};

#define X(snd, path, dlay)                                                     \
	Sound snd = {                                                              \
		.data     = snd##_DAT,                                                 \
		.data_len = sizeof(snd##_DAT),                                         \
		.delay    = dlay,                                                      \
	};

SOUNDS

#undef X

static SDL_AudioStream* stream = NULL;
static SDL_AudioSpec    spec;

static uint8_t audio_buf[4096];
static Sound   snd_queue[100]; // good enough
static size_t  snd_queue_len = 0;

void feedSounds(void*, SDL_AudioStream* stream, int required, int) {
	for(size_t i = 0; i < snd_queue_len;) {
		Sound* it          = &snd_queue[i];
		size_t can_consume = min((size_t) required, it->data_len - it->seek);

		SDL_MixAudio(
			audio_buf, it->data + it->seek, spec.format, can_consume, VOLUME
		);

		it->seek += can_consume;
		if(it->seek >= it->data_len) {
			snd_queue[i] = snd_queue[--snd_queue_len];
		} else {
			i++;
		}
	}

	SDL_PutAudioStreamData(stream, audio_buf, required);

	memset(audio_buf, 0, arrlen(audio_buf));
}

float playSound(Sound snd) {
	if(snd_queue_len >= arrlen(snd_queue)) {
		fprintf(stderr, "sound queue overflow!!!\n");
		exit(1);
	}

	snd_queue[snd_queue_len++] = snd;

	return snd.delay;
}

int initSound(void) {
	// Init sound subsystem
	if(!SDL_Init(SDL_INIT_AUDIO)) {
		dieSDL("Failed to launch audio subsystem");
	}

	// Init sounds from embedded mem
	SDL_IOStream* load_stream = NULL;

#define X(snd, path, dlay)                                                     \
	load_stream = SDL_IOFromConstMem((snd##_DAT), (sizeof(snd##_DAT)));        \
	if(!load_stream) dieSDL("Failed to load sound" #snd);                      \
	if(!SDL_LoadWAV_IO(load_stream, true, &spec, &snd.data, &snd.data_len)) {  \
		dieSDL("Failed to load sound" #snd);                                   \
	}

	SOUNDS

#undef X

	stream = SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, feedSounds, NULL
	);
	if(!stream) dieSDL("Failed to open audio device stream");

	SDL_ResumeAudioStreamDevice(stream);

	return 0;
}

#endif
