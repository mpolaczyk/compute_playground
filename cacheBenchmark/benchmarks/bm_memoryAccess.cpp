#include "benchmark/benchmark.h"
#include "../setup.h"

void BM_memoryAccess_eachElement(benchmark::State& state)
{
  const size_t size = static_cast<size_t>(state.range(0));
  const int num = size / sizeof(int);
  for (auto _ : state)
  {
    int* arr = new int[num];
    // Access each element of the array
    for (int i = 0; i < num; i++)
    {
      arr[i] *= 3;
    }
    delete arr;
  }
  state.SetComplexityN(state.range(0));
}

void BM_memoryAccess_oneElement(benchmark::State& state)
{
  const size_t size = static_cast<size_t>(state.range(0));
  const int num = size / sizeof(int);
  const int delta = CACHE_LINE_SIZE / sizeof(int);
  for (auto _ : state)
  {
    int* arr = new int[num];
    // Access one element per cache line, for the whole array
    for (int i = 0; i < num; i+=delta)
    {
      arr[i] *= 3;
    }
    delete arr;
  }
  state.SetComplexityN(state.range(0));
}