#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <mutex>
#include <atomic>
#include <future>
#include <queue>
#include <vector>
#include <functional>
#include <memory>
#include <stdexcept>

class ThreadPool
{
public:
    // 构造函数：创建指定数量的工作线程
    explicit ThreadPool(size_t max_threads = std::thread::hardware_concurrency(),
                       size_t max_tasks = 1000);
    
    // 禁止拷贝构造函数
    ThreadPool(const ThreadPool&) = delete;
    
    // 禁止赋值操作符
    ThreadPool& operator=(const ThreadPool&) = delete;
    // 提交无返回值任务
    template<class F, class... Args>
    void submit(F&& f, Args&&... args)
    {
        // 包装任务
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(mtx_);

            // 检查线程池是否已停止
            if (stop_) {
                throw std::runtime_error("Cannot submit task to stopped ThreadPool");
            }

            // 检查任务队列是否已满
            if (tasks_.size() >= max_tasks_) {
                throw std::runtime_error("ThreadPool task queue is full");
            }

            // 添加任务到队列
            tasks_.emplace([task]() { task(); });
        }
        cv_.notify_one(); // 通知一个等待的线程
    }

    // 提交有返回值任务
    template<class F, class... Args>
    auto submit_with_result(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        // 创建一个shared_ptr包装的任务，以便future能捕获结果
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(mtx_);

            // 检查线程池是否已停止
            if (stop_) {
                throw std::runtime_error("Cannot submit task to stopped ThreadPool");
            }

            // 检查任务队列是否已满
            if (tasks_.size() >= max_tasks_) {
                throw std::runtime_error("ThreadPool task queue is full");
            }

            // 添加任务到队列
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one(); // 通知一个等待的线程

        return result; // 返回future以便获取结果
    }


    // 暂停线程池
    void pause();

    // 恢复线程池
    void resume();

    // 获取当前任务队列大小
    size_t queue_size() const;

    // 获取当前活跃任务数
    size_t active_tasks() const;

    // 获取线程池大小
    size_t thread_count() const;

    // 清空任务队列
    void clear_tasks();

    // 析构函数：停止线程池
    ~ThreadPool();

private:
    std::vector<std::thread> workers_; // 工作线程池
    std::queue<std::function<void()>> tasks_; // 任务队列
    mutable std::mutex mtx_; // 互斥锁保护共享数据
    std::condition_variable cv_; // 条件变量用于线程同步
    bool stop_; // 线程池停止标志
    bool paused_; // 线程池暂停标志
    size_t max_tasks_; // 最大任务队列大小
    std::atomic<size_t> active_tasks_; // 当前活跃任务数
};


#endif // THREAD_POOL_H
