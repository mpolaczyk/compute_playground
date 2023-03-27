#include "benchmark/benchmark.h"

/*
* https://www.youtube.com/watch?v=tD5NrevFtbU&ab_channel=MollyRocket
* 
* Clean code rules
* 
* 1. Polymorphism instead of if/switch statements.
* 2. You should never know internals of something you work with.
* 3. Functions should be small.
* 4. Functions should do one thing.
* 5. Don't repeat yourself.
* 
* Give terrible performance.
* 
* Google Benchmark: https://github.com/google/benchmark
* Installed as NuGet package: gbenchmark.1.5.1
* 
* Example filter: --benchmark_filter=BM_getAreaVTBL/16384
* 
*/


#define RUN_SIMPLE 1
#define RUN_SIMD 1
#define RUN_MT 1

#define RANGE_MUL 8
#define RANGE_MIN 1<<8
#define RANGE_MAX 1<<20
#define BENCHMARK_ARGS RangeMultiplier(RANGE_MUL)->Range(RANGE_MIN, RANGE_MAX)->Complexity()->Unit(benchmark::kMicrosecond);

// Simple benchmarks - simple.cpp
#if RUN_SIMPLE
extern void BM_getAreaVTBL(benchmark::State& state);
extern void BM_getAreaSwitchStruct(benchmark::State& state);
extern void BM_getAreaCoeffArray(benchmark::State& state);
BENCHMARK(BM_getAreaVTBL)->BENCHMARK_ARGS
BENCHMARK(BM_getAreaSwitchStruct)->BENCHMARK_ARGS
BENCHMARK(BM_getAreaCoeffArray)->BENCHMARK_ARGS
#endif

// SIMD benchmark - simd.cpp
#if RUN_SIMD
extern void BM_getAreaSSE(benchmark::State& state);
extern void BM_getAreaAVX(benchmark::State& state);
extern void BM_getAreaAVX512(benchmark::State& state);
BENCHMARK(BM_getAreaSSE)->BENCHMARK_ARGS
BENCHMARK(BM_getAreaAVX)->BENCHMARK_ARGS
BENCHMARK(BM_getAreaAVX512)->BENCHMARK_ARGS
#endif

// Multi threaded benchmark - mt.cpp
#if RUN_MT
extern void BM_getAreaThreads(benchmark::State& state);
extern void BM_getAreaPPL(benchmark::State& state);
extern void BM_getAreaPool(benchmark::State& state);
BENCHMARK(BM_getAreaThreads)->BENCHMARK_ARGS
BENCHMARK(BM_getAreaPPL)->BENCHMARK_ARGS
BENCHMARK(BM_getAreaPool)->BENCHMARK_ARGS
#endif

BENCHMARK_MAIN();