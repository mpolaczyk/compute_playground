#include <future>
#include <functional>

#include "threadPool.h"

threadPool::threadPool(size_t numThreads) : stop(false)
{
  auto workerFunc = [this]() {
    while (true)
    {
      std::function<void()> job;
      {
        std::unique_lock lock(mtx);
        cv.wait(lock, [this]() { return stop || !poolJobs.empty(); });
        if (stop && poolJobs.empty())
        {
          return;
        }
        job = std::move(poolJobs.front());
        poolJobs.pop();
      }
      job();
    }
  };
  for (size_t i = 0; i < numThreads; ++i)
  {
    std::thread worker(workerFunc);
    poolWorkers.emplace_back(std::move(worker));
  }
}

threadPool::~threadPool()
{
  {
    std::unique_lock<std::mutex> lock(mtx);
    stop = true;
  }
  cv.notify_all();
  for (auto& worker : poolWorkers)
  {
    worker.join();
  }
}

std::future<float> threadPool::emplace(std::function<float(int)> func, int i)
{
  auto task = std::make_shared<std::packaged_task<float(int)>>(std::bind(func, i));

  std::future<float> result = task->get_future();
  {
    std::unique_lock lock(mtx);
    poolJobs.emplace([task]() -> void { (*task)(0); });
  }
  cv.notify_one();
  return result;
}


template <typename T, typename ... Args>
std::future<T> threadPool::emplace2(std::function<T(Args...)> func, Args... args)
{
  //using result_type = decltype(func(args...));
  //
  //auto task = std::packaged_task<result_type>(std::bind(func, args...));    // error C2027: use of undefined type 'std::packaged_task<float>' - so weird!
  //std::future<T> f = task->get_future();
  //{
  //  std::unique_lock lock(mtx);
  //  poolJobs.emplace([task = std::move(task)]() -> void { task(); });
  //}
  //cv.notify_one();
  //return f;
}