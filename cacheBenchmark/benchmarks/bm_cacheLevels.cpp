#include "benchmark/benchmark.h"
#include "../setup.h"

void BM_cacheLevels(benchmark::State& state)
{
  const int size = static_cast<int>(state.range(0));
  const int num = size / sizeof(int);
  const int steps = 64 * 1024 * 1024; // Arbitrary number of steps
  const int lengthMod = num - 1;

  for (auto _ : state)
  {
    int* arr = new int[num];
    for (int i = 0; i < steps; i++)
    {
      arr[(i * 16) & lengthMod]++; // (x & lengthMod) is equal to (x % arr.Length)
    }
    delete arr;
  }
  state.SetComplexityN(state.range(0));
}