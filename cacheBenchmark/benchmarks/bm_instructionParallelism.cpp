#include "benchmark/benchmark.h"

void BM_instructionParallelism_dependent(benchmark::State& state)
{
  const int steps = static_cast<int>(state.range(0));
  int* a = new int[2];
  for (auto _ : state)
  {
    a[0] = 0;
    benchmark::DoNotOptimize(a);
    for (int i = 0; i < steps; i++) 
    { 
      a[0] += 1; 
      a[0] += 1;
    }
  }
  delete a;
  state.SetComplexityN(state.range(0));
}

void BM_instructionParallelism_independent(benchmark::State& state)
{
  const int steps = static_cast<int>(state.range(0));
  int* a = new int[2];
  for (auto _ : state)
  {
    a[0] = 0;
    a[1] = 0;
    benchmark::DoNotOptimize(a);
    for (int i = 0; i < steps; i++)
    {
      a[0] += 1;
      a[1] += 1;
    }
  }
  delete a;
  state.SetComplexityN(state.range(0));
}