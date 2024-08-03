#include "benchmark/benchmark.h"
#include "setup.h"

/*
* Based on: Gallery of Processor Cache Effects
* by Igor Ostrovsky
* 
* https://igoro.com/archive/gallery-of-processor-cache-effects/
*/

extern void BM_memoryAccess_eachElement(benchmark::State& state);
BENCHMARK(BM_memoryAccess_eachElement)->RangeMultiplier(4)->Range(1<<10, 64<<20)->Complexity()->Unit(benchmark::kMicrosecond);

extern void BM_memoryAccess_oneElement(benchmark::State& state);
BENCHMARK(BM_memoryAccess_oneElement)->RangeMultiplier(4)->Range(1 << 10, 64 << 20)->Complexity()->Unit(benchmark::kMicrosecond);

extern void BM_instructionParallelism_dependent(benchmark::State& state);
BENCHMARK(BM_instructionParallelism_dependent)->RangeMultiplier(4)->Range(1 << 10, 64 << 10)->Complexity()->Unit(benchmark::kMicrosecond);

extern void BM_instructionParallelism_independent(benchmark::State& state);
BENCHMARK(BM_instructionParallelism_independent)->RangeMultiplier(4)->Range(1 << 10, 64 << 10)->Complexity()->Unit(benchmark::kMicrosecond);

extern void BM_cacheLevels(benchmark::State& state);
BENCHMARK(BM_cacheLevels)->RangeMultiplier(2)->Range(1024,512*1024*1024)->Complexity()->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();

