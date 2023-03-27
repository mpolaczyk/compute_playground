#include "benchmark/benchmark.h"

#include "simd.h"
#include "tools/cpuid.h"

static SIMD::shapes shapesFactorySIMD(int numShapes)
{
  SIMD::shapes ans = SIMD::shapes(numShapes);

  for (int i = 0; i < numShapes; i += 4)
  {
    ans.initAndRandomize(i, SIMD::shapeType::circle);
    ans.initAndRandomize(i + 1, SIMD::shapeType::rectangle);
    ans.initAndRandomize(i + 2, SIMD::shapeType::square);
    ans.initAndRandomize(i + 3, SIMD::shapeType::triangle);
  }
  return ans;
}

void BM_getAreaSSE(benchmark::State& state)
{
  if (!InstructionSet::SSE()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactorySIMD(numShapes);

  for (auto _ : state)
  {
    volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaSSE
    benchmark::DoNotOptimize(sum);
    sum = shapes.areaSSE();

  }
  state.SetComplexityN(state.range(0));
}

void BM_getAreaAVX(benchmark::State& state)
{
  if (!InstructionSet::AVX()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactorySIMD(numShapes);

  for (auto _ : state)
  {
    volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaAVX
    benchmark::DoNotOptimize(sum);
    sum = shapes.areaAVX();

  }
  state.SetComplexityN(state.range(0));
}

void BM_getAreaAVX512(benchmark::State& state)
{
  if (!InstructionSet::AVX512F()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactorySIMD(numShapes);

  for (auto _ : state)
  {
    volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaAVX512
    benchmark::DoNotOptimize(sum);
    sum = shapes.areaAVX512();

  }
  state.SetComplexityN(state.range(0));
}