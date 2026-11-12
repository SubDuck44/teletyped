#include "dynarr.c"
#include "utils.c"

typedef struct {
	uint8_t* data;
	uint32_t data_len; // Must be in num floats
	float    delay;
	size_t   seek;
} Sound;

ArrayN(Sound, SoundQueue);

#define SOUNDS                                                                 \
	X(SOUND_BL, "bell.wav", 0.1)                                               \
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

float playSound(Sound* snd);
int   initSound(void);

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
		.data_len = sizeof(snd##_DAT) / sizeof(float),                         \
		.delay    = dlay,                                                      \
	};

SOUNDS

#undef X

#define AUDIO_BUF_SIZE 192000 // NOTE about 1 second of 48kHz float32 audio

static SDL_AudioStream* stream = NULL;
static uint8_t          audio_buf[AUDIO_BUF_SIZE];
static SoundQueue       snd_queue;
static SDL_AudioSpec    spec;

void feedSounds(void*, const void*, int) {
	println("feeding sound data...");

	memset(audio_buf, 0, AUDIO_BUF_SIZE);

	for(size_t i = 0; i < snd_queue.len; i++) {
		Sound        snd         = snd_queue.ptr[i];
		const size_t can_consume = min(snd.data_len - snd.seek, AUDIO_BUF_SIZE);

		println("can consume: %d", (int) can_consume);

		if(!SDL_MixAudio(
			   audio_buf,           //
			   snd.data + snd.seek, //
			   spec.format,         //
			   can_consume,         //
			   1.0                  // TODO variable volume through argv
		   ))
			dieSDL("failed to mix audio");

		snd.seek += can_consume;

		if(snd.seek >= snd.data_len) {
			snd_queue.ptr[i] = snd_queue.ptr[--snd_queue.len];
		}
	}

	SDL_PutAudioStreamDataNoCopy(
		stream,                                //
		audio_buf,                             //
		AUDIO_BUF_SIZE,                        //
		snd_queue.len > 0 ? feedSounds : NULL, //
		NULL                                   //
	);

	SDL_FlushAudioStream(stream);
}

float playSound(Sound* snd) {
	// TODO inject sounds directly into the buffer, no compromises
	snd->seek = 0;
	ArrayAdd(snd_queue, *snd);
	println("playing sound, %d sounds rem", (int) snd_queue.len);

	// TODO What to do to kickstart feedSounds callback loop?
	if(snd_queue.len == 0) println("starting feed callback chain...");
	feedSounds(NULL, NULL, 0); // NOTE feedSounds doesnt use these args

	return snd->delay;
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
	/* TODO: Segfault somewhere here */                                        \
	if(!SDL_LoadWAV_IO(load_stream, true, &spec, &snd.data, &snd.data_len)) {  \
		dieSDL("Failed to load sound" #snd);                                   \
	}

	SOUNDS

#undef X

	stream = SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL
	);
	if(!stream) dieSDL("Failed to open audio device stream");

	SDL_ResumeAudioStreamDevice(stream);

	return 0;
}

#endif
