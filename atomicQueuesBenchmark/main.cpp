#include <iostream>
#include "benchmark/benchmark.h"
#include "queues.h"


extern void BM_queue_simplest(benchmark::State& state);
BENCHMARK(BM_queue_simplest)->DenseRange(4, 32, 8)->Unit(benchmark::kMillisecond);

extern void BM_queue_simplest2(benchmark::State& state);
BENCHMARK(BM_queue_simplest2)->DenseRange(4, 32, 8)->Unit(benchmark::kMillisecond);

extern bool unit_test_all_queues();

int main(int argc, char** argv)
{
    unit_test_all_queues();
    ::benchmark::Initialize(&argc, argv);
    ::benchmark::RunSpecifiedBenchmarks();
}

