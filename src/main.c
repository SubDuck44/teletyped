#include "sounds.c"
#include "utils.c"

#include <ctype.h>
#include <termios.h>

struct termios restore_attrs;

ev_io    stdin_watcher;
ev_timer stroke_watcher;

/* Gets an ASCII char and, if it is a control character, returns a string which
   which makes the character readable, i.e. 'LF' for 'line feed'.
   If c is not a control character, this function will return NULL.*/
const char* makePrintable(const char c) {
	static const char* const SYMS[] = {
		"NUL", // Null character
		"SOH", // Start of heading
		"STX", // Start of text
		"ETX", // End of text
		"EOT", // End of transmission
		"ENQ", // Enquiry
		"ACK", // Acknowledge
		"BEL", // Bell
		"BS",  // Backspace
		"HT",  // Horizontal tab
		"LF",  // Newline
		"VT",  // Vertical tab
		"FF",  // Form feed
		"CR",  // Carriage return
		"SO",  // Shift in
		"SI",  // Shift out
		"DLE", // Data link escape
		"DC1", // Device control 1
		"DC2", // Device control 2
		"DC3", // Device control 3
		"DC4", // Device control 4
		"NAK", // Negative acknowledge
		"SYN", // Synchronous idle
		"ETB", // End of transmission block
		"CAN", // Cancel
		"EM",  // End of medium
		"SUB", // Substitute
		"ESC", // Escape
		"FS",  // File separator
		"GS",  // Group separator
		"RS",  // Record separator
		"US",  // Unit separator
	};

	if(c == 127) return "DEL"; // Delete
	if((size_t) c < arrlen(SYMS)) return SYMS[(size_t) c];

	return NULL;
}

void onStdinReady(EV_P_ ev_io* w, int revents) {
	(void) w;
	(void) revents;

	char c = getchar();

	assert(c != EOF && "Got stdin ready on EOF");

	static Sound* typing_snds[] = {
		&SOUND_TY_1, &SOUND_TY_2, &SOUND_TY_3, &SOUND_TY_4, &SOUND_TY_5,
	};

	printf("Got: ");
	if(iscntrl(c)) {
		println("%s", makePrintable(c));
	} else if(c & (1 << 7)) {
		println("%hhb", c);
	} else {
		println("%c", c);

		// TODO make macro/function
		float wait = playSound(typing_snds[SDL_rand(arrlen(typing_snds))]);
		ev_timer_set(&stroke_watcher, wait, 0.0);
		ev_timer_start(loop, &stroke_watcher);

		ev_io_stop(loop, &stdin_watcher);
	}
	if(c == 'q') exit(0);
}

void onWriteheadReady(EV_P_ ev_timer* w, int revents) {
	(void) loop;
	(void) revents;
	(void) w;

	println("Ready to write!");
	ev_io_start(loop, &stdin_watcher);
}

void onExit(void) {
	// Exit raw mode
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &restore_attrs);
	////////////////////
}

int main(void) {
	// Enter raw mode
	struct termios attrs;
	tcgetattr(STDIN_FILENO, &attrs);
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
		0        //
		| ECHO   // Disable echoing on stdout
		| ICANON // Disable canonical mode
		| ISIG   // Disable C-c and C-z termination handlers
		| IEXTEN // Disable C-v on some terminals
	);

	tcsetattr(STDIN_FILENO, TCSAFLUSH, &attrs);

	atexit(onExit);

	////////////////////

	struct ev_loop* event_loop = EV_DEFAULT;

	ev_io_init(&stdin_watcher, onStdinReady, STDIN_FILENO, EV_READ);
	ev_io_start(event_loop, &stdin_watcher);

	ev_timer_init(&stroke_watcher, onWriteheadReady, 0, 0);

	// Init sounds
	initSound();

	ev_run(event_loop, 0);

	return 0;
}
