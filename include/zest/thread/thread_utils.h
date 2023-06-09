#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <functional>
#include <emmintrin.h>
#include <future>
#include <thread>

namespace Zest
{
template <typename R>
bool is_future_ready(std::future<R> const& f)
{
    return f.valid() && (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
}

template <typename T>
std::future<T> make_ready_future(T val)
{
    std::promise<T> promise;
    promise.set_value(val);
    return promise.get_future();
}

struct spin_mutex
{
    void lock() noexcept
    {
        // approx. 5x5 ns (= 25 ns), 10x40 ns (= 400 ns), and 3000x350 ns
        // (~ 1 ms), respectively, when measured on a 2.9 GHz Intel i9
        constexpr std::array iterations = { 5, 10, 3000 };

        for (int i = 0; i < iterations[0]; ++i)
        {
            if (try_lock())
                return;
        }

        for (int i = 0; i < iterations[1]; ++i)
        {
            if (try_lock())
                return;

            _mm_pause();
        }

        while (true)
        {
            for (int i = 0; i < iterations[2]; ++i)
            {
                if (try_lock())
                    return;

                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
                _mm_pause();
            }

            // waiting longer than we should, let's give other threads
            // a chance to recover
            std::this_thread::yield();
        }
    }

    bool try_lock() noexcept
    {
        return !flag.test_and_set(std::memory_order_acquire);
    }

    void unlock() noexcept
    {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

inline bool spin_mutex_try(spin_mutex& mutex, const std::function<void()>& fnCB)
{
    if (mutex.try_lock())
    {
        fnCB();
        mutex.unlock();
        return true;
    }
    else
    {
        return false;
    }
}

class spin_mutex_lock
{
public:
    spin_mutex_lock(spin_mutex & mutex)
        : m(mutex)
    {
        m.lock();
    }

    ~spin_mutex_lock()
    {
        m.unlock();
    }

private:
    spin_mutex& m;
};

} // namespace Zest
