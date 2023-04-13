#pragma once

#define RUN_SIMPLE 1
#define RUN_SIMD 1
#define RUN_ISPC 1
#define RUN_MT 1

#define USE_AVX512 0

#define INCREMENT_SSE 4
#define INCREMENT_AVX 8
#define INCREMENT_AVX512 16

#define RANGE_MUL 4
#if USE_AVX512
#define RANGE_MIN std::thread::hardware_concurrency()*INCREMENT_AVX512
#else
#define RANGE_MIN std::thread::hardware_concurrency()*INCREMENT_AVX
#endif
#if USE_AVX512
#define RANGE_MAX std::thread::hardware_concurrency()*INCREMENT_AVX512*RANGE_MUL*1024*16
#else
#define RANGE_MAX std::thread::hardware_concurrency()*INCREMENT_AVX*RANGE_MUL*1024*16
#endif

#define MAX_THREADS 64
