#pragma once

#include <array>

#include "locks.h"

template<typename ELEM, int NUM>
class mpmc_queue_simplest
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
    bool push(ELEM elem)
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

    bool pop(ELEM& out_elem)
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

// CODE BELOW
// Based on: https://www.youtube.com/watch?v=bjz_bMNNWRk
// https://github.com/erez-strauss/lockfree_mpmc_queue/blob/master/mpmc_queue.h

inline static constexpr size_t cachelinesize{ 64 };

template<typename ELEM, int NUM>
class mpmc_queue_erez_strauss
{
    using value_type = ELEM;
    using index_type = unsigned int;

    static_assert(NUM > 0 && ((NUM & (NUM - 1)) == 0), "NUM needs to be a power of two");
    static_assert(std::atomic<index_type>::is_always_lock_free, "Element type needs to be always lock-free");

    template<size_t S>
    struct unit_value;
    template<>
    struct unit_value<8>
    {
        using type = uint64_t;
    };
    struct alignas(8) helper_entry
    {
        value_type _d;
        index_type _i;
    };
    using entry_as_value = typename unit_value<sizeof(helper_entry)>::type;
    struct alignas(sizeof(helper_entry)) entry
    {
        union entry_union
        {
            mutable entry_as_value _value;
            struct entry_data
            {
                value_type _data{};
                index_type _seq{};
            } _x;
            entry_union() { _value = 0; }
        } _u;

        entry() noexcept { clear(); }
        explicit entry(index_type s) noexcept
        {
            clear();
            _u._x._seq = s;
        }
        explicit entry(index_type s, value_type d) noexcept
        {
            clear();
            _u._x._data = d;
            _u._x._seq  = s;
        }
        explicit entry(entry_as_value v) noexcept { _u._value = v; }
        ~entry() noexcept = default;
        
        void clear() noexcept { _u._value = 0; }
        
        void set_seq(index_type s) noexcept
        {
            clear();
            _u._x._seq = s;
        }
        void set(index_type s, value_type d) noexcept
        {
            clear();
            _u._x._seq  = s;
            _u._x._data = d;
        }

        index_type get_seq() noexcept { return _u._x._seq; }
        value_type get_data() noexcept { return _u._x._data; }

        bool is_empty() { return !(_u._x._seq & 1U); }
        bool is_full() { return !is_empty(); }
        bool is_empty() const { return !(_u._x._seq & 1U); }
        bool is_full() const { return !is_empty(); }

        entry& operator=(entry_as_value v) noexcept
        {
            _u._value = v;
            return *this;
        }

        entry_as_value load() noexcept
        {
            return reinterpret_cast<std::atomic<entry_as_value>*>(this)->load();
        }
        entry_as_value load() const noexcept
        {
            return reinterpret_cast<const std::atomic<entry_as_value>*>(this)->load();
        }
        
        bool compare_exchange(entry expected, entry new_value) noexcept
        {
            return reinterpret_cast<std::atomic<entry_as_value>*>(this)->compare_exchange_strong(expected._u._value, new_value._u._value);
        }
    };

    template<unsigned int N>
    class array_inplace
    {
        static_assert(N > 0 && ((N & (N - 1)) == 0), "compile time array requires N to be a power of two: 1, 2, 4, 8, 16 ...");

        std::array<entry, N> _data{};

    public:
        entry& operator[](size_t index) noexcept { return _data[index & (N - 1)]; }
        const entry& operator[](size_t index) const noexcept { return _data[index & (N - 1)]; }

        constexpr size_t size() const noexcept { return N; }
        constexpr size_t index_mask() const noexcept { return N - 1; }
        constexpr size_t capacity() const noexcept { return N; }
    };
    
    array_inplace<NUM> _array;
    std::atomic<index_type> _write_index alignas(2 * cachelinesize);
    std::atomic<index_type> _read_index alignas(2 * cachelinesize);

public:
    mpmc_queue_erez_strauss()
    {
        for (index_type i = 0; i < _array.size(); ++i)
        {
            entry& x = _array[i];
            x.set_seq(i << 1);
        }
    }

    ~mpmc_queue_erez_strauss()
    {
        value_type v;
        while (pop(v));
    }

    mpmc_queue_erez_strauss(const mpmc_queue_erez_strauss&)            = delete;
    mpmc_queue_erez_strauss(mpmc_queue_erez_strauss&&)                 = delete;
    mpmc_queue_erez_strauss& operator=(const mpmc_queue_erez_strauss&) = delete;
    mpmc_queue_erez_strauss& operator=(mpmc_queue_erez_strauss&&)      = delete;
    
    bool push(value_type d) noexcept
    {
        while (true)
        {
            index_type wr_index = _write_index.load();
            index_type seq = _array[wr_index].get_seq();

            if (seq == static_cast<index_type>(wr_index << 1))
            {
                entry e{ static_cast<index_type>(wr_index << 1) };
                entry data_entry{ static_cast<index_type>((wr_index << 1) | 1U), d };

                if (_array[wr_index].compare_exchange(e, data_entry))
                {
                    _write_index.compare_exchange_strong(wr_index, wr_index + 1);
                    return true;
                }
            }
            else if ((seq == static_cast<index_type>((wr_index << 1) | 1U)) ||
                (static_cast<index_type>(seq) == static_cast<index_type>((wr_index + _array.size()) << 1)))
            {
                _write_index.compare_exchange_strong(wr_index, wr_index + 1);
            }
            else if (static_cast<index_type>(seq + (_array.size() << 1)) ==
                static_cast<index_type>((wr_index << 1) | 1U))
            {
                return false;
            }
        }
    }

    bool pop(value_type& d) noexcept
    {
        index_type rd_index = _read_index.load();

        while (true)
        {
            entry e{_array[rd_index].load()};
            if (e.get_seq() == static_cast<index_type>((rd_index << 1) | 1U))
            {
                entry empty_entry{static_cast<index_type>((rd_index + _array.size()) << 1U)};

                if (_array[rd_index].compare_exchange(e, empty_entry))
                {
                    d = e.get_data();
                    _read_index.compare_exchange_strong(rd_index, rd_index + 1);
                    return true;
                }
            }
            else if (static_cast<index_type>(e.get_seq() | 1U) ==
                     static_cast<index_type>(((rd_index + _array.size()) << 1) | 1U))
            {
                _read_index.compare_exchange_strong(rd_index, rd_index + 1);
            }
            else if (e.get_seq() == static_cast<index_type>(rd_index << 1))
            {
                return false;
            }
            rd_index = _read_index.load();
        }
    }
};

