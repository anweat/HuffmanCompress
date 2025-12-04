#include "task/bmpHandler.h"
#include "task/hufHandler.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    // 设置测试文件路径
    std::string inputBmp = "test.bmp";
    std::string outputHuf = "test_output.huf";
    std::string outputBmp = "test_restored.bmp";
    
    if (argc > 1) {
        inputBmp = argv[1];
    }
    if (argc > 2) {
        outputHuf = argv[2];
    }
    if (argc > 3) {
        outputBmp = argv[3];
    }
    
    std::cout << "=== Huffman Compression Test ===" << std::endl;
    std::cout << "Input BMP: " << inputBmp << std::endl;
    std::cout << "Output HUF: " << outputHuf << std::endl;
    std::cout << "Restored BMP: " << outputBmp << std::endl;
    
    try {
        // 1. 测试 BMP 到 HUF 的压缩
        std::cout << "\n1. 测试压缩 (BMP -> HUF)..." << std::endl;
        hufHandler huf;
        if (!huf.bmp2huf_start(inputBmp, outputHuf,nullptr)) {
            std::cerr << "错误：压缩过程失败！" << std::endl;
            return 1;
        }
        std::cout << "压缩成功！" << std::endl;
        
        // 2. 测试 HUF 到 BMP 的解压缩
        std::cout << "\n2. 测试解压缩 (HUF -> BMP)..." << std::endl;
        bmpHandler bmp;
        if (!bmp.huf2bmp_start(outputHuf, outputBmp,nullptr)) {
            std::cerr << "错误：解压缩过程失败！" << std::endl;
            return 1;
        }
        std::cout << "解压缩成功！" << std::endl;
        
        std::cout << "\n=== 测试完成 ===" << std::endl;
        std::cout << "请检查输出文件是否正常。" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "异常：" << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
