

#include <iostream>

#include <numeric>
#include <thread>
#include <algorithm>
#include <future>

#include <ppl.h>

#include "benchmark/benchmark.h"

#include "tools/simd.h"
#include "tools/cpuid.h"
#include "tools/threadPool.h"
#include "tools/shapesFactory.h"
#include "setup.h"

// Reuse SIMD implementation to squeeze more from the CPU!
#if USE_AVX512
#define AREA_FUNC areaAVX512
#define FLAG_FUNC AVX512F
#else 
#define AREA_FUNC areaAVX
#define FLAG_FUNC AVX
#endif


void BM_getArea_AVX_Threads(benchmark::State& state)
{
  assert(std::thread::hardware_concurrency() <= MAX_THREADS);

  if (!InstructionSet::FLAG_FUNC()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  // Split work
  int numThreads = std::thread::hardware_concurrency();

  std::vector<int> indexes(numThreads); // index of the first shape in each chunk
  int shapesPerThread = numShapes / numThreads;
  for (int i = 0; i < numThreads; i++)
  {
    indexes[i] = i * shapesPerThread;
  }

  // Define work
  volatile float sum = 0.0f;
  for (auto _ : state)
  {
    benchmark::DoNotOptimize(sum);
    sum = 0.0f;
    // Work split into chunks, one chunk per thread
    // + Multi threaded and with SIMD, theoretically the best option
    // - great example of thread creation overhead!
    // - mutex overhead
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    std::array<float, MAX_THREADS> results;
    for (int i = 0; i < MAX_THREADS; i++)
    {
      results[i] = 0.0f;
    }

    auto workLambda = [&shapes, &sum, &results, shapesPerThread, numShapes](int threadIndex, int shapeIndex)
    {
      float parialSum = SIMD::AREA_FUNC(shapes, shapeIndex, std::min(shapeIndex + shapesPerThread, numShapes));
      {
        results[threadIndex] = parialSum;
      }
    };

    // Do work
    for (int i = 0; i < numThreads; i++)
    {
      threads.push_back(std::thread(workLambda, i, indexes[i]));
    }
    for (int i = 0; i < numThreads; i++)
    {
      threads[i].join();
    }
    // Sum up
    for (int i = 0; i < numThreads; i++)
    {
      sum += results[i];
    }
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}


void BM_getArea_AVX_PPL(benchmark::State& state)
{
  if (!InstructionSet::FLAG_FUNC()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

  volatile float sum = 0.0f;
  for (auto _ : state)
  {
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
        return SIMD::AREA_FUNC(shapes, shapeIndex, std::min(shapeIndex + shapesPerThread, numShapes));
      });

    // Sum up
    sum = std::accumulate(begin(results), end(results), 0.0f);
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}


#include "cvmarkersobj.h"
Concurrency::diagnostic::marker_series series;

void BM_getArea_AVX_ThreadPool(benchmark::State& state)
{
  if (!InstructionSet::FLAG_FUNC()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactory::instance().getCache();

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
  volatile float sum = 0.0f;
  benchmark::DoNotOptimize(sum);
  std::function<float(int)> workFunc = [&shapes, &sumMutex, &sum, shapesPerThread, numShapes](int shapeIndex) -> float
  {
    return SIMD::AREA_FUNC(shapes, shapeIndex, shapeIndex + shapesPerThread < numShapes ? shapeIndex + shapesPerThread : numShapes);
  };

  // Create pool
  threadPool pool(numThreads);
  
  for (auto _ : state)
  {
    sum = 0.0f;
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
  }
  shapesFactory::instance().validateResult(numShapes, sum);
  state.SetComplexityN(state.range(0));
}

