#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>

#include "benchmark/benchmark.h"

inline void validate_result(benchmark::State& state, int value)
{
    if (state.thread_index == 0)
    {
        const long long expected = state.iterations() * state.threads;
        if (expected != value)
        {
            std::cout << "Wrong result, expected: " << expected << " got " << value << std::endl;
        }
    }
}

int counter = 0;
void BM_add(benchmark::State& state)
{
    counter = 0;
    for (auto _ : state)
    {
        counter += 1;
    }
    // Obviously, result is wrong, but let's keep it for reference
}

std::atomic_int atomic_counter = 0;
void BM_atomic_add(benchmark::State& state)
{
    atomic_counter.store(0);
    for (auto _ : state)
    {
        atomic_counter.fetch_add(1);
    }

    validate_result(state, atomic_counter.load());
}


int mutex_counter = 0;
std::mutex mutex;
void BM_mutex_add(benchmark::State& state)
{
    mutex_counter = 0;
    for (auto _ : state)
    {
        std::lock_guard<std::mutex> scope_guard(mutex);
        mutex_counter += 1;
    }
    validate_result(state, mutex_counter);
}


// Test-and-set
class tas_spin_lock
{
private:
    std::atomic_flag atomic_flag = ATOMIC_FLAG_INIT;

public:
    void lock()
    {
        while (atomic_flag.test_and_set(std::memory_order_acquire)) 
        {
            _mm_pause();
        }
    }
    void unlock()
    {
        atomic_flag.clear(std::memory_order_release);
    }
};
int tas_spin_lock_counter = 0;
tas_spin_lock tas_lock;
void BM_tas_spin_lock_add(benchmark::State& state)
{
    tas_spin_lock_counter = 0;
    for (auto _ : state)
    {
        tas_lock.lock();
        tas_spin_lock_counter += 1;
        tas_lock.unlock();
    }
    validate_result(state, tas_spin_lock_counter);
}

// Test, test-and-set
class ttas_spin_lock
{
private:
    std::atomic<bool> atomic_bool = { false };

public:
    void lock()
    {
        for (;;)
        {
            // Optimistically assume the lock is free on the first try
            if (!atomic_bool.exchange(true, std::memory_order_acquire)) // returns value before exchange
            {
                return;
            }
            // Wait for lock to be released without generating cache misses
            // Read only operation does not invalidate the cache (cache coherency protocol)
            while (atomic_bool.load(std::memory_order_relaxed))
            {
                _mm_pause();
            }
        }
    }
    void unlock()
    {
        atomic_bool.store(false, std::memory_order_release);
    }
};
int ttas_spin_lock_counter = 0;
ttas_spin_lock ttas_lock;
void BM_ttas_spin_lock_add(benchmark::State& state)
{
    ttas_spin_lock_counter = 0;
    for (auto _ : state)
    {
        ttas_lock.lock();
        ttas_spin_lock_counter += 1;
        ttas_lock.unlock();
    }
    validate_result(state, ttas_spin_lock_counter);
}

// Compare exchange
/* exchanged = atomic_object.compare_exchange_strong(expected, desired)
              
   bool compare_exchange_strong(expected, desired)
   {
     if(atomic_object == expected)
     {
       atomic_object = desired;
       return true;
     }
     expected = atomic_object; 
     return false;
   }
*/
class ces_spin_lock
{
private:
    std::atomic<bool> atomic_bool = ATOMIC_FLAG_INIT;

public:
    void lock()
    {
        bool expected = false;
        for(;;)
        {           
            if(atomic_bool.compare_exchange_strong(expected, true, std::memory_order_acquire))
            {
                return;
            }
            else
            {
                _mm_pause();
                expected = false;
            }
        }
    }
    void unlock ()
    {
        atomic_bool.store(false, std::memory_order_release);
    }
};
int ces_spin_lock_counter = 0;
ces_spin_lock ces_lock;
void BM_ces_spin_lock_add(benchmark::State& state)
{
    ces_spin_lock_counter = 0;
    for (auto _ : state)
    {
        ces_lock.lock();
        ces_spin_lock_counter += 1;
        ces_lock.unlock();
    }
    validate_result(state, ces_spin_lock_counter);
}