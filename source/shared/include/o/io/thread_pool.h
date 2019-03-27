#pragma once

#include <boost/asio/io_context.hpp>
#include <functional>
#include <future>
#include <iostream>
#include <thread>
#include <vector>

class thread_pool {

  public:
    thread_pool(size_t threads) : thread_count_(threads) {}

    boost::asio::io_context& ioc() { return executor_; }

    void start() {
        executor_.get_executor().on_work_started();
        for (auto i = 0; i < thread_count_; ++i)
            threads_.emplace_back([this]() { this->executor_.run(); });
    }

    void stop() {
        executor_.get_executor().on_work_finished();
        for (auto& thread : threads_)
            if (thread.joinable()) thread.join();
    }

    std::vector<std::thread>& threads() { return threads_; }

    size_t thread_count_;
    std::vector<std::thread> threads_;
    boost::asio::io_context executor_;
};
