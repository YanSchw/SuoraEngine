#pragma once

#include <thread>
#include <future>
#include <mutex>

namespace Suora
{
    template<typename R>
    static bool IsFutureReady(std::future<R> const& future)
    {
        return future.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
    }

}