#include <iostream>
#include <fstream>
#include <string>
#include "task/bmpHandler.h"
#include "task/hufHandler.h"

// 比较两个文件是否完全相同
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::ifstream f1(file1, std::ios::binary);
    std::ifstream f2(file2, std::ios::binary);
    
    if (!f1 || !f2) {
        std::cerr << "无法打开文件" << std::endl;
        return false;
    }
    
    std::string content1((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    std::string content2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());
    
    f1.close();
    f2.close();
    
    return content1 == content2;
}

int main() {
    std::string original_file = "D:\\Desktop\\bmp\\test1.bmp";
    std::string compressed_file = "test1_compressed.huf";
    std::string decompressed_file = "test1_decompressed.bmp";
    
    std::cout << "测试BMP压缩和解压缩...\n";
    
    // 压缩
    if (!hufHandler::bmp2huf_start(original_file, compressed_file, nullptr)) {
        std::cerr << "✗ 压缩失败\n";
        return 1;
    }
    std::cout << "✓ 压缩完成\n";
    
    // 解压缩
    if (!bmpHandler::huf2bmp_start(compressed_file, decompressed_file, nullptr)) {
        std::cerr << "✗ 解压缩失败\n";
        return 1;
    }
    std::cout << "✓ 解压缩完成\n";
    
    // 比较文件
    if (compareFiles(original_file, decompressed_file)) {
        std::cout << "✓ 文件相同 - 测试通过\n";
    } else {
        std::cout << "✗ 文件不同 - 测试失败\n";
        
        // 输出文件大小信息
        std::ifstream f1(original_file, std::ios::binary | std::ios::ate);
        std::ifstream f2(decompressed_file, std::ios::binary | std::ios::ate);
        
        std::cout << "原始大小: " << f1.tellg() << " 字节\n";
        std::cout << "解压大小: " << f2.tellg() << " 字节\n";
        
        f1.close();
        f2.close();
    }
    
    return 0;
}