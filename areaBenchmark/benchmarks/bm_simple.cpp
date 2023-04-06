#include <vector>

#include "benchmark/benchmark.h"

#include "tools/shapesFactory.h"


void BM_getAreaVTBL(benchmark::State& state)
{
  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getObjectOriented();

  float sum = 0;
  for (auto _ : state)   // Alternative: while (state.KeepRunning())
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);     // force result to be stored in either memory or register
    for (int i = 0; i < numShapes; i++)
    {
      // Performance overhead: 
      // - Vector of pointers to structs, spread across wide memory area, cache misses
      // - Indirection to vtable with offset based on the function name, then indirection to a function, than call
      // - No inline
      // - SSE used to calculate single float value
      sum += shapes[i]->area();
    }
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}


void BM_getAreaSwitchStruct(benchmark::State& state)
{
  using namespace shapes::switchStruct;

  int numShapes = static_cast<int>(state.range(0));
  std::vector<shape> shapes = shapesFactory::instance().getSwitchStruct();

  float sum = 0;
  for (auto _ : state)
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);
    for (int i = 0; i < numShapes; i++)
    {
      // Performance:
      // + No vtable, no indirection
      // + Size of any struct is known, vector is a continuous block of memory. Better despite sizeof(struct) being bigger.
      // + area() inlined
      // - Switch statement costs but less than vtable indirection
      // - SSE used to calculate single float values
      sum += shapes[i].area();
    }
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}


void BM_getAreaCoeffArray(benchmark::State& state)
{
  using namespace shapes::coeffArray;

  int numShapes = static_cast<int>(state.range(0));
  std::vector<shape> shapes = shapesFactory::instance().getCoeffArray();

  float sum = 0;
  for (auto _ : state)
  {
    sum = 0.0f;
    benchmark::DoNotOptimize(sum);
    for (int i = 0; i < numShapes; i++)
    {
      // Performance:
      // + No switch statement costs but less than vtable indirection
      // - SSE used to calculate single float values
      sum += shapes[i].area();
    }
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}