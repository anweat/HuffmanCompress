#ifndef SUBMIT_CONVERT_TASK_H
#define SUBMIT_CONVERT_TASK_H
#include "../FileTaskPool/threadPool.h" // 使用改进后的线程池
#include "bmpHandler.h"
#include "hufHandler.h"
#include <functional>

// 全局线程池对象（懒加载，线程安全）
inline ThreadPool &gPool()
{
    static ThreadPool pool(std::thread::hardware_concurrency());
    return pool;
}

// 提交HUF到BMP转换任务
inline std::future<bool> submit_huf2bmp(const std::string &input_path, const std::string &output_path, double *progress)
{
    Logger::getInstance().info("提交HUF到BMP转换任务");
    return gPool().submit_with_result([input_path, output_path, progress]() {
        bmpHandler handler;
        return handler.huf2bmp_start(input_path, output_path, progress);
    });
}

// 提交BMP到HUF转换任务
inline std::future<bool> submit_bmp2huf(const std::string &input_path, const std::string &output_path, double *progress)
{
    Logger::getInstance().info("提交BMP到HUF转换任务");
    return gPool().submit_with_result([input_path, output_path, progress]() {
        hufHandler handler;
        return handler.bmp2huf_start(input_path, output_path, progress);
    });
}

// 获取线程池状态信息的辅助函数
inline size_t getThreadPoolQueueSize()
{
    return gPool().queue_size();
}

inline size_t getThreadPoolActiveTasks()
{
    return gPool().active_tasks();
}

inline size_t getThreadPoolThreadCount() { return gPool().thread_count(); }

#endif