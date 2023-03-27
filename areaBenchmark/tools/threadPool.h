#pragma once


#include <queue>
#include <thread>
#include <vector>


class threadPool
{
public:
  threadPool(const threadPool&) = delete;
  threadPool(threadPool&&) = delete;
  threadPool& operator=(const threadPool&) = delete;
  threadPool& operator=(threadPool&&) = delete;

  threadPool(size_t numThreads);
  ~threadPool();

  std::future<float> emplace(std::function<float(int)>, int);

  template <class T, class... Args>
  std::future<T> emplace2(std::function<T(Args...)> func, Args... args);
  
private:

  std::mutex mtx;
  std::queue<std::function<void()>> poolJobs;
  bool stop;

  std::vector<std::thread> poolWorkers;
  std::condition_variable cv;
  
};

