#include <assert.h>
#include <err.h>
#include <unistd.h>

#include <SDL3/SDL.h>

////////////////////

// TODO get args on runtime
#define MAX_WIDTH 80
#define RIGHT_MARGIN 10
#define TYPING_SPEED 1.0
#define VOLUME 1.0

////////////////////

#define die(fmt, ...)                                                          \
	err(1, "%s:%d: " fmt, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#define dieSDL(fmt, ...)                                                       \
	die(fmt ": %s", __VA_ARGS__ __VA_OPT__(, ) SDL_GetError())

#define arrlen(arr) (sizeof(arr) / sizeof(*(arr)))

#define println(fmt, ...) printf(fmt "\r\n" __VA_OPT__(, ) __VA_ARGS__);

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min3(x, y, z) min(min(x, y), z)
#define max3(x, y, z) max(max(x, y), z)
