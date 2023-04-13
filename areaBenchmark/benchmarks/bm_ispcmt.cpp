

#include "benchmark/benchmark.h"

#include "benchmarks/getAreaMT.h"
#include "tools/cpuid.h"
#include "tools/shapesFactory.h"
#include "tools/simd.h"


void BM_getArea_AVX_ISPC_MT(benchmark::State& state)
{
  if (!InstructionSet::AVX()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  volatile float sum = 0.0f;
  for (auto _ : state)
  {
    benchmark::DoNotOptimize(sum);
    sum = ispc::getAreaMT(shapes.va.data(), shapes.vb.data(), shapes.vcoeff.data(), numShapes, 256);
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));

}