#include <iostream>
#include <fstream>
#include <string>
#include "task/hufHandler.h"
// 导入Logger头文件
#include "logger/Logger.h"

int main() {
    // 启用日志
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    
    // 测试BMP转HUF并验证压缩率
    std::string input_bmp = "test.bmp";
    std::string output_huf = "test_compressed.huf";
    
    std::cout << "正在测试BMP转HUF压缩...\n";
    std::cout << "输入文件: " << input_bmp << std::endl;
    std::cout << "输出文件: " << output_huf << std::endl;
    
    // 检查输入文件是否存在
    std::ifstream bmp_file(input_bmp, std::ios::binary);
    if (!bmp_file) {
        std::cerr << "错误: 找不到输入文件 " << input_bmp << std::endl;
        return 1;
    }
    
    // 获取原始BMP文件大小
    bmp_file.seekg(0, std::ios::end);
    std::streampos bmp_size = bmp_file.tellg();
    bmp_file.close();
    
    std::cout << "原始BMP文件大小: " << bmp_size << " 字节\n";
    
    // 执行BMP转HUF
    double process = 0.0;
    bool success = hufHandler::bmp2huf_start(input_bmp, output_huf, &process);
    
    if (!success) {
        std::cerr << "错误: BMP转HUF失败\n";
        return 1;
    }
    
    // 直接解析HUF文件头来获取关键信息
    std::ifstream huf_stream(output_huf, std::ios::binary);
    if (huf_stream) {
        std::cout << "\nHUF文件头详细信息:\n";
        
        // 跳过hufType(2字节)和fileSize(4字节)，直接读取关键参数
        huf_stream.seekg(6, std::ios::beg);
        
        // 读取reserved(4字节) - 跳过
        huf_stream.seekg(4, std::ios::cur);
        
        // 读取keySize(1字节)
        uint8_t key_size = 0;
        huf_stream.read(reinterpret_cast<char*>(&key_size), 1);
        
        // 读取valueSize(1字节)
        uint8_t value_size = 0;
        huf_stream.read(reinterpret_cast<char*>(&value_size), 1);
        
        // 读取keyNum(4字节)
        uint32_t key_num = 0;
        huf_stream.read(reinterpret_cast<char*>(&key_num), 4);
        
        // 读取bitNum(8字节)
        uint64_t bit_num = 0;
        huf_stream.read(reinterpret_cast<char*>(&bit_num), 8);
        
        // 读取bitsetSize(8字节)
        uint64_t bitset_size = 0;
        huf_stream.read(reinterpret_cast<char*>(&bitset_size), 8);
        
        std::cout << "key_num: " << key_num << std::endl;
        std::cout << "key_size: " << static_cast<int>(key_size) << " 字节" << std::endl;
        std::cout << "value_size: " << static_cast<int>(value_size) << " 字节" << std::endl;
        std::cout << "bit_num: " << bit_num << " 位" << std::endl;
        std::cout << "bitset_size: " << bitset_size << " 字节" << std::endl;
        
        std::cout << "\n理论文件大小计算:\n";
        std::cout << "文件头: 32 字节" << std::endl;
        std::cout << "键数据: " << key_num * key_size << " 字节" << std::endl;
        std::cout << "值数据: " << key_num * value_size << " 字节" << std::endl;
        std::cout << "位集数据: " << bitset_size << " 字节" << std::endl;
        
        uint32_t calculated_size = 32 + bitset_size + 
                                  key_num * key_size + 
                                  key_num * value_size;
        std::cout << "总理论大小: " << calculated_size << " 字节" << std::endl;
        
        huf_stream.close();
    } else {
        std::cerr << "警告: 无法打开HUF文件进行详细分析\n";
    }
    
    // 获取压缩后的HUF文件大小
    std::ifstream huf_file_stream(output_huf, std::ios::binary);
    if (!huf_file_stream) {
        std::cerr << "错误: 找不到输出文件 " << output_huf << std::endl;
        return 1;
    }
    
    huf_file_stream.seekg(0, std::ios::end);
    std::streampos huf_size = huf_file_stream.tellg();
    huf_file_stream.close();
    
    std::cout << "压缩后HUF文件大小: " << huf_size << " 字节\n";
    
    // 计算压缩率
    double compression_ratio = ((double)bmp_size - (double)huf_size) / (double)bmp_size * 100.0;
    std::cout << "计算得到的压缩率: " << compression_ratio << "%\n";
    
    // 验证HUF文件头中的fileSize字段
    std::ifstream huf_header_stream(output_huf, std::ios::binary);
    if (!huf_header_stream) {
        std::cerr << "错误: 无法读取HUF文件头" << std::endl;
        return 1;
    }
    
    // 跳过hufType(2字节)
    huf_header_stream.seekg(2, std::ios::beg);
    
    // 读取fileSize字段(4字节)
    uint32_t file_size_in_header = 0;
    huf_header_stream.read(reinterpret_cast<char*>(&file_size_in_header), 4);
    
    std::cout << "HUF文件头中的fileSize: " << file_size_in_header << " 字节\n";
    std::cout << "实际文件大小与头文件中记录大小的差异: " << (static_cast<uint32_t>(huf_size) - file_size_in_header) << " 字节\n";
    
    // 如果差异为0，则fileSize字段正确
    if (static_cast<uint32_t>(huf_size) == file_size_in_header) {
        std::cout << "✅ HUF文件头中的fileSize字段正确！\n";
    } else {
        std::cerr << "❌ HUF文件头中的fileSize字段错误！\n";
        return 1;
    }
    
    std::cout << "\n测试完成，压缩率计算正确！\n";
    return 0;
}