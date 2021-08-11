#pragma once

#include <chrono>

namespace stlr {
    class Timer{
    private:
        std::chrono::time_point<std::chrono::steady_clock> start_time_point;
        std::chrono::time_point<std::chrono::steady_clock> stop_time_point;
        double elapsed_time;
        bool running;

    public:
        ///
        /// \brief Is the timer running?
        /// \return True if the timer has started and is currently running.
        ///
        constexpr bool is_running() const noexcept {
            return running;
        }

        ///
        /// \brief The elapsed time in milliseconds.
        /// \return The amount of time between start and stopping the timer.
        ///
        constexpr double get_elapsed_time() const noexcept {
            return elapsed_time;
        }

        ///
        /// \brief Starts the timer.
        ///
        void start() noexcept;

        ///
        /// \brief Stops the timer.
        ///
        void stop() noexcept;

    private:
        ///
        /// \brief Calculates the amount of time between the start and stop of the timer.
        ///
        void calculate_elapsed_time() noexcept;
    };
}
