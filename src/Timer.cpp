 #include "Timer.hpp"

namespace stlr {
    void Timer::start() noexcept {
        start_time_point = std::chrono::steady_clock::now();
        running = true;
    }

    void Timer::stop() noexcept {
        stop_time_point = std::chrono::steady_clock::now();
        running = false;
        calculate_elapsed_time();
    }

    void Timer::calculate_elapsed_time() noexcept {
        auto diff = stop_time_point - start_time_point;
        elapsed_time = diff.count();
    }
}
