#define _DEFAULT_SOURCE

#include <SDL3/SDL.h>
#include <assert.h>
#include <err.h>
#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//====================
#include "dynarr.c"

#define die(fmt, ...)                                                          \
	err(1, "%s:%d: " fmt, __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#define SDL_Die(fmt, ...)                                                      \
	die(fmt ": %s", __VA_ARGS__ __VA_OPT__(, ) SDL_GetError())

#define arrlen(arr) (sizeof(arr) / sizeof(*(arr)))

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))
#define min3(x, y, z) min(min(x, y), z)
#define max3(x, y, z) max(max(x, y), z)
