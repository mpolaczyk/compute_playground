
#include "benchmark/benchmark.h"
#include "benchmarks/getArea.h"
#include "tools/cpuid.h"
#include "tools/simd.h"
#include "tools/shapesFactory.h"

void BM_getArea_AVX_ISPC(benchmark::State& state)
{
  if (!InstructionSet::AVX()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  volatile float sum = 0.0f;
  for (auto _ : state)
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);
    sum = ispc::getArea(shapes.va.data(), shapes.vb.data(), shapes.vcoeff.data(), 0, numShapes);
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
    
}