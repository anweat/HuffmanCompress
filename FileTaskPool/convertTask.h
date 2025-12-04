#ifndef CONVERTTASK_H
#define CONVERTTASK_H

#include <thread>
#include <atomic>
#include <future>
#include <functional>
#include <string>
#include <iostream>
#include <stdexcept>

// 通用转换任务模板
// 使用函数指针或可调用对象来定义转换逻辑
// InType: 输入文件类型
// OutType: 输出文件类型
template <typename InType, typename OutType>
struct ConvertTask
{
    using ConvertFunction = std::function<OutType*(std::string path, double* progress)>;
    using LoaderFunction = std::function<InType*(std::string path)>;
    using SaverFunction = std::function<bool(OutType* data, std::string path)>;

    std::thread worker; // 仅当"非线程池"模式时用
    std::atomic<double> progress{0.0}; // 任务进度
    std::promise<OutType*> result; // 任务结果
    
    // 转换函数，用于执行实际的转换逻辑
    ConvertFunction convertFunc;
    
    // 可选的加载和保存函数
    LoaderFunction loaderFunc;
    SaverFunction saverFunc;

    /* 真正干活的函数，静态放到线程池里跑 */
    static void run(std::string path,
                    double *externalProgress, // 主线程给的进度条地址
                    std::promise<OutType*> prom,
                    ConvertFunction convertFunc) {
        try {
            if (!convertFunc) {
                prom.set_exception(std::make_exception_ptr(std::runtime_error("Convert function not provided")));
                return;
            }
            
            // 执行转换
            OutType* result = convertFunc(path, externalProgress);
            
            // 设置结果
            prom.set_value(result);
            
            // 最终进度
            if (externalProgress) *externalProgress = 1.0;
        } catch (const std::exception& e) {
            std::cerr << "Error in conversion: " << e.what() << std::endl;
            prom.set_exception(std::current_exception());
        }
    }
    
    // 构造函数，接收转换函数
    explicit ConvertTask(ConvertFunction func) : convertFunc(func) {}
    
    // 带加载器和保存器的构造函数
    ConvertTask(ConvertFunction func, LoaderFunction loader, SaverFunction saver)
        : convertFunc(func), loaderFunc(loader), saverFunc(saver) {}
    
    // 启动转换任务（非线程池模式）
    void start(std::string path) {
        if (!convertFunc) {
            throw std::runtime_error("Convert function not set");
        }
        
        worker = std::thread([this, path]() {
            try {
                OutType* resultData = convertFunc(path, &progress);
                this->result.set_value(resultData);
                progress = 1.0;
            } catch (...) {
                this->result.set_exception(std::current_exception());
            }
        });
    }
    
    // 获取结果的future
    std::future<OutType*> getFuture() {
        return result.get_future();
    }
    
    // 等待任务完成
    void wait() {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    // 工厂方法：创建一个转换任务实例
    static ConvertTask create(ConvertFunction func) {
        return ConvertTask(func);
    }
};

#endif // CONVERTTASK_H


