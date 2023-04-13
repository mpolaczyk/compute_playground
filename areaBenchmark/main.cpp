#include "benchmark/benchmark.h"
#include "setup.h"

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
* Dependencies: 
* 
* 1. Google Benchmark: https://github.com/google/benchmark
* Installed as NuGet package: gbenchmark.1.5.1
* Example filter: --benchmark_filter=BM_getAreaVTBL/16384
* 
* 2. Intel® Implicit SPMD Program Compiler: https://ispc.github.io/index.html
* Library and headers are included in the project.
* Compiler needs to be installed in your system. 
* - Download https://github.com/ispc/ispc/releases/download/v1.19.0/ispc-v1.19.0-windows.zip
* - Add ispc-v1.19.0-windows\bin to PATH so that ispc.exe can be executed
* - Follow https://ispc.github.io/ispc.html#getting-started-with-ispc
*/

#include <thread>

#define BENCHMARK_ARGS RangeMultiplier(RANGE_MUL)->Range(RANGE_MIN, RANGE_MAX)->Complexity()->Unit(benchmark::kMicrosecond);

// Simple benchmarks - simple.cpp
#if RUN_SIMPLE
extern void BM_getArea_OOP(benchmark::State& state);
extern void BM_getArea_SwitchStruct(benchmark::State& state);
extern void BM_getArea_CoeffArray(benchmark::State& state);
BENCHMARK(BM_getArea_OOP)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_SwitchStruct)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_CoeffArray)->BENCHMARK_ARGS
#endif

// SIMD benchmark - simd.cpp
#if RUN_SIMD
extern void BM_getArea_SSE(benchmark::State& state);
extern void BM_getArea_AVX(benchmark::State& state);
#if USE_AVX512
extern void BM_getArea_AVX512(benchmark::State& state);
#endif
BENCHMARK(BM_getArea_SSE)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_AVX)->BENCHMARK_ARGS
#if USE_AVX512
BENCHMARK(BM_getArea_AVX512)->BENCHMARK_ARGS
#endif
#endif

// Multi threaded benchmark - mt.cpp
#if RUN_MT
extern void BM_getArea_OOP_PPL(benchmark::State& state);
extern void BM_getArea_AVX_Threads(benchmark::State& state);
extern void BM_getArea_AVX_ThreadPool(benchmark::State& state);
extern void BM_getArea_AVX_PPL(benchmark::State& state);
BENCHMARK(BM_getArea_OOP_PPL)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_AVX_Threads)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_AVX_ThreadPool)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_AVX_PPL)->BENCHMARK_ARGS
#endif

// ISPC benchmark - ispc.cpp
#if RUN_ISPC
extern void BM_getArea_AVX_ISPC(benchmark::State& state);
extern void BM_getArea_AVX_ISPC_MT(benchmark::State& state);
BENCHMARK(BM_getArea_AVX_ISPC)->BENCHMARK_ARGS
BENCHMARK(BM_getArea_AVX_ISPC_MT)->BENCHMARK_ARGS
#endif

BENCHMARK_MAIN();

