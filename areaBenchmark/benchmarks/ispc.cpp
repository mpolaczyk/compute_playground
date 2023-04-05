
#include "benchmark/benchmark.h"
#include "benchmarks/getArea.h"
#include "tools/cpuid.h"
#include "simd.h"

static SIMD::shapes shapesFactoryISPC(int numShapes)
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

void BM_getAreaISPC(benchmark::State& state)
{
  if (!InstructionSet::AVX()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactoryISPC(numShapes);

  for (auto _ : state)
  {
    volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaAVX
    benchmark::DoNotOptimize(sum);
    sum = ispc::getArea(shapes.a.data(), shapes.b.data(), shapes.coeff.data(), numShapes);

  }
  state.SetComplexityN(state.range(0));
    
}