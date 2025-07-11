#pragma once

#include <atomic>
#include <windows.h>

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

template<typename T>
class lock_scope
{
public:
    lock_scope() = delete;
    lock_scope(T* in_lock)
    {
        lock = in_lock;
        lock->lock();
    }
    ~lock_scope()
    {
        lock->unlock();
    }
private:
    T* lock;
};