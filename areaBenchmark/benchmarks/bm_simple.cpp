#include <vector>
#include <thread>
#include <ppl.h>

#include "benchmark/benchmark.h"

#include "tools/shapesFactory.h"
#include <numeric>


void BM_getArea_OOP(benchmark::State& state)
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

void BM_getArea_OOP_PPL(benchmark::State& state)
{
  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getObjectOriented();

  volatile float finalSum = 0.0f;
  for (auto _ : state)
  {
    benchmark::DoNotOptimize(finalSum);

    // Performance overhead: 
    // + Multi threaded OOP, fastest approach to MT picked
    // - All mentioned in BM_getAreaVTBL

    // Split work
    int numThreads = std::thread::hardware_concurrency();
    std::vector<int> indexes(numThreads); // index of the first shape in each chunk
    int shapesPerThread = numShapes / numThreads;
    for (int i = 0; i < numThreads; i++)
    {
      indexes[i] = i * shapesPerThread;
    }

    // Run
    std::vector<float> results(numThreads);
    concurrency::parallel_transform(begin(indexes), end(indexes), begin(results),
      [&](int shapeIndex)
      {
        volatile float sum = 0.0f;
        for (int i = shapeIndex; i < std::min(shapeIndex + shapesPerThread, numShapes); i++)
        {
          sum += shapes[i]->area();
        }
        return sum;
      });

    // Sum up
    finalSum = std::accumulate(begin(results), end(results), 0.0f);
  }
  shapesFactory::instance().validateResult(numShapes, finalSum);
  state.SetComplexityN(state.range(0));
}


void BM_getArea_SwitchStruct(benchmark::State& state)
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


void BM_getArea_CoeffArray(benchmark::State& state)
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