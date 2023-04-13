
#include <immintrin.h>    // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
#include <assert.h>

#include "benchmark/benchmark.h"

#include "tools/simd.h"
#include "tools/cpuid.h"
#include "tools/shapesFactory.h"


void BM_getArea_SSE(benchmark::State& state)
{
  if (!InstructionSet::SSE()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  volatile float sum = 0.0f;
  for (auto _ : state)
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);
    sum = SIMD::areaSSE(shapes, 0, numShapes);
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}


void BM_getArea_AVX(benchmark::State& state)
{
  if (!InstructionSet::AVX()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  volatile float sum = 0.0f;
  for (auto _ : state)
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);
    sum = SIMD::areaAVX(shapes, 0, numShapes);
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}


void BM_getArea_AVX512(benchmark::State& state)
{
  if (!InstructionSet::AVX512F()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  volatile float sum = 0.0f;
  for (auto _ : state)
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);
    sum = SIMD::areaAVX512(shapes, 0, numShapes);
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}