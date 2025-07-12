#pragma once

#include <array>

#include "locks.h"

// CRTP, static polymorphism for fun
template<typename QUEUE, typename ELEM>
class queue_interface
{
    bool push_back(ELEM elem)
    {
        return static_cast<QUEUE*>(this)->push_back();
    }
    bool pop_front(ELEM& elem)
    {
        return static_cast<QUEUE*>(this)->pop_front(elem);
    }
};


template<typename ELEM, int NUM>
class mpmc_queue_simplest : public queue_interface<mpmc_queue_simplest<ELEM, NUM>, ELEM>
{
    // Static size, continuous memory
    // Index based
    // Spin lock on both read and write operations

private:
    ttas_spin_lock lock{};
    std::array<ELEM, NUM> data{};
    int front_index{ 0 };
    int back_index{ 0 };
    int size{ 0 };

public:
    bool push_back(ELEM elem)
    {
        lock_scope x(&lock);
        if(size<NUM)
        {
            size++;
            data[back_index] = elem;
            back_index++;
            back_index = back_index % NUM;
            return true;
        }
        return false;
    }

    bool pop_front(ELEM& out_elem)
    {
        lock_scope x(&lock);
        if(size>0)
        {
            out_elem = data[front_index];
            front_index++;
            front_index = front_index % NUM;
            size--;
            return true;
        }
        return false;
    }
};


template<typename ELEM, int NUM>
class mpmc_queue_simplest2 : public queue_interface<mpmc_queue_simplest2<ELEM, NUM>, ELEM>
{
    // Static size, continuous memory
    // Index based
    // Spin lock on both read and write operations

private:
    ttas_spin_lock lock{};
    std::array<ELEM, NUM> data{};
    int front_index{ 0 };
    int back_index{ 0 };
    int size{ 0 };

public:
    bool push_back(ELEM elem)
    {
        lock_scope x(&lock);
        if (size < NUM)
        {
            size++;
            data[back_index] = elem;
            back_index++;
            back_index = back_index % NUM;
            return true;
        }
        return false;
    }

    bool pop_front(ELEM& out_elem)
    {
        lock_scope x(&lock);
        if (size > 0)
        {
            out_elem = data[front_index];
            front_index++;
            front_index = front_index % NUM;
            size--;
            return true;
        }
        return false;
    }
};