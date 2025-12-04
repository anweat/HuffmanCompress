#include "threadPool.h"
#include <iostream>

// 构造函数实现
ThreadPool::ThreadPool(size_t max_threads, size_t max_tasks)
    : stop_(false), paused_(false), max_tasks_(max_tasks), active_tasks_(0)
{
    if (max_threads == 0) {
        max_threads = 1; // 至少创建一个线程
    }

    for (size_t i = 0; i < max_threads; ++i) {
        workers_.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mtx_);
                    // 等待条件：有任务且未暂停，或者停止
                    cv_.wait(lock, [this] {
                        return (!tasks_.empty() && !paused_) || stop_;
                    });

                    // 如果停止且任务队列为空，则退出线程
                    if (stop_ && tasks_.empty()) {
                        return;
                    }

                    // 如果暂停或队列为空，则继续等待
                    if (paused_ || tasks_.empty()) {
                        continue;
                    }

                    // 取出任务
                    task = std::move(tasks_.front());
                    tasks_.pop();
                    active_tasks_++;
                }

                try {
                    task(); // 执行任务
                } catch (const std::exception& e) {
                    // 记录异常但不中断线程
                    std::cerr << "Thread pool task exception: " << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "Thread pool task unknown exception" << std::endl;
                }

                // 任务完成，减少活动任务计数
                active_tasks_--;
            }
        });
    }
}

// 暂停线程池
void ThreadPool::pause() {
    std::unique_lock<std::mutex> lock(mtx_);
    paused_ = true;
}

// 恢复线程池
void ThreadPool::resume() {
    {
        std::unique_lock<std::mutex> lock(mtx_);
        paused_ = false;
    }
    cv_.notify_all(); // 通知所有等待的线程
}

// 获取当前任务队列大小
size_t ThreadPool::queue_size() const {
    std::unique_lock<std::mutex> lock(mtx_);
    return tasks_.size();
}

// 获取当前活跃任务数
size_t ThreadPool::active_tasks() const {
    return active_tasks_;
}

// 获取线程池大小
size_t ThreadPool::thread_count() const {
    return workers_.size();
}

// 清空任务队列
void ThreadPool::clear_tasks() {
    std::unique_lock<std::mutex> lock(mtx_);
    std::queue<std::function<void()>> empty;
    std::swap(tasks_, empty);
}

// 析构函数实现
ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(mtx_);
        stop_ = true;
        paused_ = false; // 确保线程能够退出
    }
    cv_.notify_all(); // 通知所有线程
    for (auto &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}