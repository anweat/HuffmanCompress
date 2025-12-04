#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include "task/bmpHandler.h"
#include "task/hufHandler.h"

// 读取文件内容到vector
std::vector<unsigned char> readFileToVector(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return {};
    }
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(file)), 
                                     std::istreambuf_iterator<char>());
}

// 比较两个文件并输出差异
bool compareFiles(const std::string& file1, const std::string& file2) {
    std::vector<unsigned char> content1 = readFileToVector(file1);
    std::vector<unsigned char> content2 = readFileToVector(file2);
    
    if (content1.empty() || content2.empty()) {
        return false;
    }
    
    std::cout << "=== 文件比较信息 ===" << std::endl;
    std::cout << "原始文件大小: " << content1.size() << " 字节" << std::endl;
    std::cout << "解压缩文件大小: " << content2.size() << " 字节" << std::endl;
    
    if (content1.size() != content2.size()) {
        std::cout << "文件大小不匹配!" << std::endl;
    }
    
    // 找出不匹配的位置
    size_t min_size = std::min(content1.size(), content2.size());
    size_t mismatch_count = 0;
    const size_t max_mismatches = 20; // 最多显示20个不匹配位置
    
    std::cout << "\n=== 差异位置信息 (最多显示20个) ===" << std::endl;
    for (size_t i = 0; i < min_size; ++i) {
        if (content1[i] != content2[i]) {
            mismatch_count++;
            if (mismatch_count <= max_mismatches) {
                std::cout << "位置 0x" << std::hex << std::setw(8) << std::setfill('0') << i 
                          << std::dec << ": 原始=0x" << std::hex << std::setw(2) << std::setfill('0') 
                          << static_cast<int>(content1[i]) << ", 解压缩=0x" 
                          << std::setw(2) << static_cast<int>(content2[i]) << std::dec << std::endl;
            }
        }
    }
    
    if (mismatch_count > max_mismatches) {
        std::cout << "... 还有 " << (mismatch_count - max_mismatches) << " 个差异未显示" << std::endl;
    }
    
    if (mismatch_count == 0) {
        std::cout << "文件完全相同!" << std::endl;
        return true;
    } else {
        std::cout << "\n总共有 " << mismatch_count << " 个差异" << std::endl;
        return false;
    }
}

// 比较BMP文件的像素数据差异
bool compareBMPPixelData(const std::string& file1, const std::string& file2) {
    // 加载BMP文件
    bmp* bmp1 = bmpHandler::load(file1);
    bmp* bmp2 = bmpHandler::load(file2);
    
    if (!bmp1 || !bmp2) {
        std::cerr << "无法加载BMP文件" << std::endl;
        if (bmp1) delete bmp1;
        if (bmp2) delete bmp2;
        return false;
    }
    
    std::cout << "\n=== BMP像素数据比较 ===" << std::endl;
    std::cout << "原始文件像素数量: " << bmp1->bit_num << std::endl;
    std::cout << "解压缩文件像素数量: " << bmp2->bit_num << std::endl;
    std::cout << "原始文件数据大小: " << bmp1->filemap.size() << " 字节" << std::endl;
    std::cout << "解压缩文件数据大小: " << bmp2->filemap.size() << " 字节" << std::endl;
    
    bool result = true;
    if (bmp1->bit_num != bmp2->bit_num) {
        std::cout << "像素数量不匹配!" << std::endl;
        result = false;
    }
    
    if (bmp1->filemap.size() != bmp2->filemap.size()) {
        std::cout << "像素数据大小不匹配!" << std::endl;
        result = false;
    }
    
    // 比较像素数据
    size_t min_size = std::min(bmp1->filemap.size(), bmp2->filemap.size());
    size_t mismatch_count = 0;
    const size_t max_mismatches = 20;
    
    std::cout << "\n=== BMP像素差异位置 (最多显示20个) ===" << std::endl;
    for (size_t i = 0; i < min_size; ++i) {
        if (bmp1->filemap[i] != bmp2->filemap[i]) {
            mismatch_count++;
            if (mismatch_count <= max_mismatches) {
                std::cout << "像素位置 " << i << ": 原始=0x" << std::hex << std::setw(2) << std::setfill('0') 
                          << static_cast<int>(bmp1->filemap[i]) << ", 解压缩=0x" 
                          << std::setw(2) << static_cast<int>(bmp2->filemap[i]) << std::dec << std::endl;
                
                // 显示周围几个像素的值
                if (i > 2 && i < min_size - 2) {
                    std::cout << "  周围像素: [";
                    for (int j = -2; j <= 2; ++j) {
                        if (j != 0) {
                            std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') 
                                      << static_cast<int>(bmp1->filemap[i+j]) << "/";
                            std::cout << "0x" << std::setw(2) << static_cast<int>(bmp2->filemap[i+j]);
                        } else {
                            std::cout << "**CURRENT**";
                        }
                        if (j < 2) std::cout << ", ";
                    }
                    std::cout << std::dec << "]" << std::endl;
                }
            }
        }
    }
    
    if (mismatch_count > max_mismatches) {
        std::cout << "... 还有 " << (mismatch_count - max_mismatches) << " 个差异未显示" << std::endl;
    }
    
    if (mismatch_count == 0) {
        std::cout << "BMP像素数据完全相同!" << std::endl;
    } else {
        std::cout << "\n总共有 " << mismatch_count << " 个像素差异" << std::endl;
        result = false;
    }
    
    delete bmp1;
    delete bmp2;
    return result;
}

// 检查HUF文件的元数据
void checkHUFMetadata(const std::string& huf_filename) {
    std::cout << "\n=== HUF文件元数据检查 ===" << std::endl;
    huf* hufFile = hufHandler::load(huf_filename);
    
    if (!hufFile) {
        std::cerr << "无法加载HUF文件" << std::endl;
        return;
    }
    
    std::cout << "HUF文件元数据: " << std::endl;
    std::cout << "  key_size: " << static_cast<int>(hufFile->key_size) << std::endl;
    std::cout << "  value_size: " << static_cast<int>(hufFile->value_size) << std::endl;
    std::cout << "  key_num: " << hufFile->key_num << std::endl;
    std::cout << "  bit_num: " << hufFile->bit_num << std::endl;
    std::cout << "  bitset_size: " << hufFile->bitset_size << std::endl;
    std::cout << "  key_value_data大小: " << hufFile->key_value_data.size() << std::endl;
    std::cout << "  bitset数据大小: " << hufFile->bitset.size() << std::endl;
    
    // 显示前几个键值对
    std::cout << "\n  前5个键值对: " << std::endl;
    int count = 0;
    for (const auto& pair : hufFile->key_value_data) {
        if (count >= 5) break;
        std::cout << "    0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(pair.first) << ": " << std::dec << pair.second << std::endl;
        count++;
    }
    
    delete hufFile;
}

int main() {
    std::string original_file = "D:\\Desktop\\bmp\\test1.bmp";
    std::string compressed_file = "test1_compressed.huf";
    std::string decompressed_file = "test1_decompressed.bmp";
    
    std::cout << "=== Huffman压缩解压缩测试 ===" << std::endl;
    std::cout << "原始文件: " << original_file << std::endl;
    std::cout << "压缩文件: " << compressed_file << std::endl;
    std::cout << "解压缩文件: " << decompressed_file << std::endl;
    
    // 1. 压缩文件
    std::cout << "\n1. 开始压缩文件..." << std::endl;
    bool compress_success = hufHandler::bmp2huf_start(original_file, compressed_file, nullptr);
    
    if (!compress_success) {
        std::cerr << "压缩失败!" << std::endl;
        return 1;
    }
    std::cout << "压缩成功!" << std::endl;
    
    // 检查HUF文件元数据
    checkHUFMetadata(compressed_file);
    
    // 2. 解压缩文件
    std::cout << "\n2. 开始解压缩文件..." << std::endl;
    bool decompress_success = bmpHandler::huf2bmp_start(compressed_file, decompressed_file, nullptr);
    
    if (!decompress_success) {
        std::cerr << "解压缩失败!" << std::endl;
        return 1;
    }
    std::cout << "解压缩成功!" << std::endl;
    
    // 3. 比较文件内容
    std::cout << "\n3. 比较原始文件和解压缩文件内容..." << std::endl;
    compareFiles(original_file, decompressed_file);
    
    // 4. 专门比较BMP像素数据
    std::cout << "\n4. 专门比较BMP像素数据..." << std::endl;
    compareBMPPixelData(original_file, decompressed_file);
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return 0;
}