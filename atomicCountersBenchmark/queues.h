#pragma once

#include <array>

#include "locks.h"

// Queue - fifo
// Static size, continuous memory, value based, index 
template<typename T, int N>
class basic_queue
{
public:
    bool push_back(T elem)
    {
        lock_scope x(&lock);
        if(size<N)
        {
            size++;
            data[back] = elem;
            back++;
            back = back % N;
            return true;
        }
        return false;
    }

    bool pop_front(T& out_elem)
    {
        lock_scope x(&lock);
        if(size>0)
        {
            int front = back - size;
            if(front < 0)
            {
                front += N;    
            }
            out_elem = data[front];
            size--;
            return true;
        }
        return false;
    }
    
private:
    ttas_spin_lock lock{};
    std::array<T, N> data{};
    int back = 0;   // last 
    int size = 0;
};
