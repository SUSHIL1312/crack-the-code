/**
 * thread_pool.cpp
 * 
 * Production-Quality Thread Pool:
 * - Worker threads with RAII std::jthread
 * - Generic task submission returning std::future<T>
 * - Clean shutdown draining pending tasks
 * - Multi-threaded stress test with verification
 */

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <cassert>

class ThreadPool {
private:
    std::vector<std::jthread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_ = false;

public:
    explicit ThreadPool(size_t threads) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        cv_.wait(lock, [this]() {
                            return stop_ || !tasks_.empty();
                        });

                        if (stop_ && tasks_.empty()) {
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        // workers_ join automatically via std::jthread
    }

    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;

        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
                return func(std::forward<Args>(args)...);
            }
        );

        std::future<ReturnType> fut = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_) {
                throw std::runtime_error("Cannot submit task to stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return fut;
    }
};

void run_tests() {
    std::cout << "[TEST] ThreadPool Task Submission & Futures Resolution...\n";

    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 20; ++i) {
        results.push_back(pool.submit([i]() {
            return i * i;
        }));
    }

    for (int i = 0; i < 20; ++i) {
        int res = results[i].get();
        assert(res == i * i);
    }

    std::cout << "  ✓ 20 tasks executed concurrently and futures verified.\n";
}

int main() {
    run_tests();
    std::cout << "✅ Capstone 05 Thread Pool PASSED!\n";
    return 0;
}
