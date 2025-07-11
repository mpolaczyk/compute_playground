#include <iostream>
#include "benchmark/benchmark.h"
#include "queues.h"

constexpr int min_threads = 1;
constexpr int max_threads = 64;

extern void BM_add(benchmark::State& state);
BENCHMARK(BM_add)->ThreadRange(min_threads, max_threads)->Unit(benchmark::kNanosecond)->UseRealTime();

extern void BM_atomic_add(benchmark::State& state);
BENCHMARK(BM_atomic_add)->ThreadRange(min_threads, max_threads)->Unit(benchmark::kNanosecond)->UseRealTime();

extern void BM_mutex_add(benchmark::State& state);
BENCHMARK(BM_mutex_add)->ThreadRange(min_threads, max_threads)->Unit(benchmark::kNanosecond)->UseRealTime();

extern void BM_tas_spin_lock_add(benchmark::State& state);
BENCHMARK(BM_tas_spin_lock_add)->ThreadRange(min_threads, max_threads)->Unit(benchmark::kNanosecond)->UseRealTime();

extern void BM_ttas_spin_lock_add(benchmark::State& state);
BENCHMARK(BM_ttas_spin_lock_add)->ThreadRange(min_threads, max_threads)->Unit(benchmark::kNanosecond)->UseRealTime();

extern void BM_ces_spin_lock_add(benchmark::State& state);
BENCHMARK(BM_ces_spin_lock_add)->ThreadRange(min_threads, max_threads)->Unit(benchmark::kNanosecond)->UseRealTime();


// TODO move to a separate project
extern void BM_queue(benchmark::State& state);
BENCHMARK(BM_queue)->DenseRange(2, 64, 2)->Unit(benchmark::kMillisecond);

extern bool unit_test_all_queues();

int main(int argc, char** argv)
{
    unit_test_all_queues();
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}

