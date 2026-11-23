#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "sounds.c"

struct termios restore_attrs;

#define PICK(arr) *(arr)[SDL_rand(arrlen(arr))]

static Sound* typing_snds[] = {
	&SOUND_TY_1, //
	&SOUND_TY_2, //
	&SOUND_TY_3, //
	&SOUND_TY_4, //
	&SOUND_TY_5, //
};

static Sound* carriage_snds[] = {
	&SOUND_CR_1, //
	&SOUND_CR_2, //
	&SOUND_CR_3, //
	&SOUND_CR_4, //
};

static Sound* whitespace_snds[] = {
	&SOUND_WS_1, //
	&SOUND_WS_2, //
	&SOUND_WS_3, //
	&SOUND_WS_4, //
};

static Sound* hitstop_snds[] = {
	&SOUND_HS_1, //
	&SOUND_HS_2, //
	&SOUND_HS_3, //
	&SOUND_HS_4, //
};

/* Gets an ASCII char and, if it is a control character, returns a string which
   which makes the character readable, i.e. 'LF' for 'line feed'.
   If c is not a control character, this function will return NULL.*/
const char* makePrintable(const char c) {
	static const char* const SYMS[] = {
		"NUL",  // Null character
		"SOH",  // Start of heading
		"STX",  // Start of text
		"ETX",  // End of text
		"EOT",  // End of transmission
		"ENQ",  // Enquiry
		"ACK",  // Acknowledge
		"BEL",  // Bell
		"BS",   // Backspace
		"    ", // Horizontal tab
		"LF",   // Newline
		"VT",   // Vertical tab
		"FF",   // Form feed
		"CR",   // Carriage return
		"SO",   // Shift in
		"SI",   // Shift out
		"DLE",  // Data link escape
		"DC1",  // Device control 1
		"DC2",  // Device control 2
		"DC3",  // Device control 3
		"DC4",  // Device control 4
		"NAK",  // Negative acknowledge
		"SYN",  // Synchronous idle
		"ETB",  // End of transmission block
		"CAN",  // Cancel
		"EM",   // End of medium
		"SUB",  // Substitute
		"ESC",  // Escape
		"FS",   // File separator
		"GS",   // Group separator
		"RS",   // Record separator
		"US",   // Unit separator
	};

	if(c == 127) return "DEL"; // Delete
	if((size_t) c < arrlen(SYMS)) return SYMS[(size_t) c];

	return NULL;
}

static int stdfd;

void onExit(void) {
	// Exit raw mode
	tcsetattr(stdfd, TCSAFLUSH, &restore_attrs);
}

void playAndDelay(Sound sound) {
	usleep(playSound(sound) * 1e6);
}


void* ttyHandler(void*) {
	int tty = open("/dev/tty", O_RDONLY);
	if(tty < 0) die("tty");

	char buf[256];
	for(;;) {
		ssize_t len = read(tty, buf, sizeof(buf));
		for(ssize_t i = 0; i < len; i++) {
			switch(buf[i]) {
				case '\x03': // C-c
				case '\x11': // C-q
					exit(0);
			}
		}
	}
}

int main(void) {
	if(isatty(STDIN_FILENO)) {
		stdfd = STDIN_FILENO;
	} else {
		stdfd = STDOUT_FILENO;

		pthread_t thread = {0};
		if(pthread_create(&thread, NULL, ttyHandler, NULL) != 0)
			die("failed to create tty handler thread");
	}

	// Enter raw mode
	struct termios attrs;
	tcgetattr(stdfd, &attrs);
	restore_attrs = attrs;

	attrs.c_iflag &= ~(
		0       //
		| ICRNL // Map C-m to CR
		| IXON  // Disable C-q/C-s flow control
	);
	attrs.c_oflag &= ~(
		0       //
		| OPOST // Don't map LF to CRLF on stdout
	);
	attrs.c_lflag &= ~(
		0      //
		| ECHO // Disable echoing on stdout
		| ICANON // Disable canonical mode
		| ISIG   // Disable C-c and C-z termination handlers
		| IEXTEN // Disable C-v on some terminals
	);

	tcsetattr(stdfd, TCSAFLUSH, &attrs);

	setbuf(stdout, NULL);

	atexit(onExit);

	////////////////////

	// Init sounds
	initSound();

	int col = 0;

	for(;;) {
		char c = getchar();
		if(c == EOF || c == '\x11' /* C-q */) {
			exit(0);
		} else if(c == '\n' || c == '\r') {
			Sound  snd      = PICK(carriage_snds);
			double fraction = min((double) col / MAX_WIDTH, 1.0);
			size_t span     = snd.delay * SECOND_TO_BYTES;
			snd.seek        = span * (1 - fraction);
			snd.seek -= snd.seek % sizeof(float);
			playSound(snd);

			printf("\n");
			while(col-- > 0) {
				printf("[D");
				usleep(1e6 / MAX_WIDTH);
			}
			printf("[G");
		} else if(c == ' ') {
			playAndDelay(PICK(whitespace_snds));
			printf(" ");
			col++;
		} else if(iscntrl(c)) {
			const char* s = makePrintable(c);
			printf("%s", s);
			col += strlen(s);
		} else {
			playAndDelay(PICK(typing_snds));
			printf("%c", c);
			col++;
		}

		if(col == MAX_WIDTH - RIGHT_MARGIN) playSound(SOUND_BL);
	}

	return 0;
}
