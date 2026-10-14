#pragma once
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
typedef ptrdiff_t ssize_t;
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
static inline uint32_t ldl_le_p(const void *p) { uint32_t v; memcpy(&v, p, 4); return v; }
static inline void stl_le_p(void *p, uint32_t v) { memcpy(p, &v, 4); }
