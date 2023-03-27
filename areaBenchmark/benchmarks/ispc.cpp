
#include "benchmark/benchmark.h"
#include "benchmarks/getArea.h"


void BM_getAreaISPC(benchmark::State& state)
{
  float vin[16], vout[16];
  for (int i = 0; i < 16; ++i)
    vin[i] = i;

  ispc::getArea(vin, vout, 16);

  for (int i = 0; i < 16; ++i)
    printf("%d: simple(%f) = %f\n", i, vin[i], vout[i]);
}