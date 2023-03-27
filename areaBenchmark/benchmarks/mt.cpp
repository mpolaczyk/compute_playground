

#include <iostream>

#include <numeric>
#include <thread>
#include <algorithm>
#include <future>

#include <ppl.h>

#include "benchmark/benchmark.h"

#include "simd.h"
#include "tools/cpuid.h"
#include "tools/threadPool.h"




// Reuse SIMD implementation to squeeze more from the CPU!
#define USE_AVX512 0
#if USE_AVX512
#define AREA_FUNC areaAVX512
#define FLAG_FUNC AVX512F
#else 
#define AREA_FUNC areaAVX
#define FLAG_FUNC AVX
#endif

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

void BM_getAreaThreads(benchmark::State& state)
{
  if (!InstructionSet::FLAG_FUNC()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactorySIMD(numShapes);

  // Split work
  int numThreads = std::thread::hardware_concurrency();
  std::vector<int> indexes(numThreads); // index of the first shape in each chunk
  int shapesPerThread = numShapes / numThreads;
  std::mutex sumMutex;
  for (int i = 0; i < numThreads; i++)
  {
    indexes[i] = i * shapesPerThread;
  }

  // Define work
  volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaSSE
  benchmark::DoNotOptimize(sum);
  auto workLambda = [&shapes, &sumMutex, &sum, shapesPerThread, numShapes](int shapeIndex)
  {
    float parialSum = shapes.AREA_FUNC(shapeIndex, std::min(shapeIndex + shapesPerThread, numShapes));
    {
      const std::lock_guard<std::mutex> lock(sumMutex);
      sum += parialSum;
    }
  };

  for (auto _ : state)
  {
    // Work split into chunks, one chunk per thread
    // + Multi threaded and with SIMD, theoretically the best option
    // - great example of thread creation overhead!
    // - mutex overhead
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    // Do work
    for (int i = 0; i < numThreads; i++)
    {
      threads.push_back(std::thread(workLambda, indexes[i]));
    }
    for (int i = 0; i < numThreads; i++)
    {
      threads[i].join();
    }
    sum = 0.0f;
  }
  state.SetComplexityN(state.range(0));
}


void BM_getAreaPPL(benchmark::State& state)
{
  if (!InstructionSet::FLAG_FUNC()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactorySIMD(numShapes);

  for (auto _ : state)
  {
    volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaSSE
    benchmark::DoNotOptimize(sum);

    // Work split into chunks, one chunk per thread
    // + Multi threaded and with SIMD, theoretically the best option
    // - single core calculation is faster for small datasets
    // - is inserting to the vector thread safe? (mutex overhead)
    // - a ton of context switching (concurrency::affinity_partitioner does not help and increases time)
     
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
        return shapes.AREA_FUNC(shapeIndex, std::min(shapeIndex + shapesPerThread, numShapes));
      });

    // Sum up
    sum = std::accumulate(begin(results), end(results), 0.0f);
  }
  state.SetComplexityN(state.range(0));
}





#include "cvmarkersobj.h"
Concurrency::diagnostic::marker_series series;

void BM_getAreaPool(benchmark::State& state)
{
  if (!InstructionSet::FLAG_FUNC()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactorySIMD(numShapes);

  // Split work
  int numThreads = std::thread::hardware_concurrency();
  std::vector<int> indexes(numThreads); // index of the first shape in each chunk
  int shapesPerThread = numShapes / numThreads;
  std::mutex sumMutex;
  for (int i = 0; i < numThreads; i++)
  {
    indexes[i] = i * shapesPerThread;
  }

  // Define work
  volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaSSE
  benchmark::DoNotOptimize(sum);
  std::function<float(int)> workFunc = [&shapes, &sumMutex, &sum, shapesPerThread, numShapes](int shapeIndex) -> float
  {
    return shapes.AREA_FUNC(shapeIndex, shapeIndex + shapesPerThread < numShapes ? shapeIndex + shapesPerThread : numShapes);
  };

  // Create pool
  threadPool pool(numThreads);
  
  for (auto _ : state)
  {
    Concurrency::diagnostic::span s2(series, _T("poolLoop"));

    std::vector<std::future<float>> results;
    for (int i = 0; i < numThreads; ++i)
    {
      std::future<float> future = pool.emplace(workFunc, indexes[i]);

      results.emplace_back(std::move(future));
    }
    
    for (auto& x : results)
    {
      sum += x.get();
    }

    sum = 0.0f;
  }
  state.SetComplexityN(state.range(0));
}

