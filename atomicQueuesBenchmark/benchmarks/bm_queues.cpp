#include <future>
#include <iostream>
#include <thread>
#include <mutex>

#include "benchmark/benchmark.h"
#include "../queues.h"


bool unit_test_queue()
{
    // Test: Push 15 elements through a queue of size 5 on one thread.
    // Check if all are there, order should be the same
    std::vector<int> input{1,2,3,4,5, 6,7,8,9,10, 11,12,13,14,15};
    std::vector<int> output{};
    output.reserve(input.size());
    int push_index = 0;
    mpmc_queue_simplest<int, 5> q;
    int value = 0;
    for(int i = 0; i <3; i++)
    {
        // Push until full
        while(push_index < input.size() && q.push(input[push_index]))
        {
            push_index++;
        }
        // Pop until empty
        while(q.pop(value))
        {
            output.push_back(value);
        }
    }
    if(input != output) return false;
    return true;
}

template<typename Q, int COUNT>
struct queue_tester
{
    int num_threads;
    Q queue;
    
    std::vector<std::thread> producer_threads;
    std::vector<std::thread> consumer_threads;
    
    std::atomic<int> detached{0};
    int pad1[15];
    std::atomic<int> finished{0};
    int pad2[15];
    std::atomic<int> sum_pushed{0};
    int pad3[15];
    std::atomic<int> sum_popped{0};
    int pad4[15];

    void producer_func(int thread_id)
    {
        // Start all threads at once
        detached.fetch_add(1);
        while(detached.load() != num_threads){ _mm_pause(); }

        // Do work
        for(int i = 0; i < COUNT; i++)
        {
            const int value = thread_id + 1;
            while(!queue.push(value)) { _mm_pause(); }
            sum_pushed.fetch_add(value);
        }

        // Report work done
        finished.fetch_add(1);
    }

    void consumer_func(int thread_id)
    {
        // Start all threads at once
        detached.fetch_add(1);
        while(detached.load() != num_threads){ _mm_pause(); }

        // Do work
        for(int i = 0; i < COUNT; i++)
        {
            int value = -1;
            while(!queue.pop(value)) { _mm_pause(); }
            if(value == -1)
            {
                std::cout << "invalid argument" << std::endl;
            }
            else
            {
                sum_popped.fetch_add(value);
            }
        }

        // Report work done
        finished.fetch_add(1);
    }

    bool run(const std::string& name, int in_num_threads, bool silent = false)
    {
        num_threads = in_num_threads;
#if _DEBUG
        if(!silent) std::cout << "Testing " << name << " threads " << num_threads << " count " << COUNT;
#endif
        for (int i = 0; i < num_threads /2; i++)
        {
            producer_threads.emplace_back(std::thread(&queue_tester::producer_func, this, i));
            consumer_threads.emplace_back(std::thread(&queue_tester::consumer_func, this, i));
        }
        for (int i = 0; i < num_threads /2; i++)
        {
            producer_threads[i].detach();
            consumer_threads[i].detach();
        }
        // Wait for work to be done
        while (finished.load() != num_threads)
        {
#if _DEBUG
            if (!silent) std::cout << ".";
#endif
            _mm_pause();
        }
#if _DEBUG
        if (!silent) std::cout << " pushed " << sum_pushed.load() << " popped " << sum_popped.load() << std::endl;
#endif
        bool success = sum_pushed.load() == sum_popped.load();
        if (!success)
        {
            if (!silent) std::cout << "FAILED!" << std::endl;
        }
        return success;
    }
};

bool unit_test_queue_mpmc()
{
    // Perform 3 tests for each queue
    // - Long queue, two threads - chill out case, don't saturate the queue
    // - Short queue, a lot of threads - to test saturation
    // - Mix

    // mpmc_queue_simplest
    const int hc = std::thread::hardware_concurrency();
    {   
        queue_tester<mpmc_queue_simplest<int, 128>, 20> qt;
        if (!qt.run("chill case", 2)) return false;
    }
    {
        queue_tester<mpmc_queue_simplest<int, 2>, 100> qt;
        if (!qt.run("stress case", hc * 2)) return false;
    }
    {
        queue_tester<mpmc_queue_simplest<int, 16>, 100> qt;
        if (!qt.run("mix case", hc)) return false;
    }
    // mpmc_queue_simplest2
    {
        queue_tester<mpmc_queue_erez_strauss<int, 128>, 20> qt;
        if (!qt.run("chill case", 2)) return false;
    }
    {
        queue_tester<mpmc_queue_erez_strauss<int, 2>, 100> qt;
        if (!qt.run("stress case", hc * 2)) return false;
    }
    {
        queue_tester<mpmc_queue_erez_strauss<int, 16>, 100> qt;
        if (!qt.run("mix case", hc)) return false;
    }
    return true;
}

bool unit_test_all_queues()
{
    if (!unit_test_queue())
    {
        std::cout << "unit_test_queue failed" << std::endl;
        return false;
    }
    if (!unit_test_queue_mpmc())
    {
        std::cout << "unit_test_queue_mpmc failed" << std::endl;
        return false;
    }
    return true;
}

void BM_queue_simplest(benchmark::State& state)
{    
    for (auto _ : state)
    {
        const int num_threads = static_cast<int>(state.range(0));
        queue_tester<mpmc_queue_simplest<int, 16>, 100> qt;
        qt.run("", num_threads, true);
    }
}

void BM_queue_erez_strauss(benchmark::State& state)
{
    for (auto _ : state)
    {
        const int num_threads = static_cast<int>(state.range(0));
        queue_tester<mpmc_queue_erez_strauss<int, 16>, 100> qt;
        qt.run("", num_threads, true);
    }
}