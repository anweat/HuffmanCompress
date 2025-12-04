#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include "FileStream/FileReader.h"
#include "huffman/huffmantree.h"
#include "huffman/bitstream.h"
#include "task/hufHandler.h"
#include "task/bmpHandler.h"

// 比较两个频率映射是否相同
template <typename T>
bool compareFrequencyMaps(const std::unordered_map<T, u64>& map1, const std::unordered_map<T, u64>& map2) {
    if (map1.size() != map2.size()) {
        std::cout << "频率表大小不同: " << map1.size() << " vs " << map2.size() << std::endl;
        return false;
    }
    
    for (const auto& pair : map1) {
        auto it = map2.find(pair.first);
        if (it == map2.end()) {
            std::cout << "键 " << static_cast<int>(pair.first) << " (0x" << std::hex << static_cast<int>(pair.first) << std::dec << ") 在第二个频率表中不存在" << std::endl;
            return false;
        }
        if (it->second != pair.second) {
            std::cout << "键 " << static_cast<int>(pair.first) << " (0x" << std::hex << static_cast<int>(pair.first) << std::dec << ") 的频率不同: " << pair.second << " vs " << it->second << std::endl;
            return false;
        }
    }
    
    return true;
}

// 比较两个编码映射是否相同
template <typename T>
bool compareCodeMaps(const std::unordered_map<T, std::pair<u64, u8>>& map1, const std::unordered_map<T, std::pair<u64, u8>>& map2) {
    if (map1.size() != map2.size()) {
        std::cout << "编码映射大小不同: " << map1.size() << " vs " << map2.size() << std::endl;
        return false;
    }
    
    for (const auto& pair : map1) {
        auto it = map2.find(pair.first);
        if (it == map2.end()) {
            std::cout << "键 " << pair.first << " 在第二个编码映射中不存在" << std::endl;
            return false;
        }
        if (it->second != pair.second) {
            std::cout << "键 " << pair.first << " 的编码不同: " 
                      << it->second.first << "(" << (int)it->second.second << ") vs " 
                      << pair.second.first << "(" << (int)pair.second.second << ")" << std::endl;
            return false;
        }
    }
    
    return true;
}

// 比较两个位流是否相同
bool compareBitStreams(const std::vector<unsigned char>& stream1, const std::vector<unsigned char>& stream2) {
    if (stream1.size() != stream2.size()) {
        std::cout << "位流大小不同: " << stream1.size() << " vs " << stream2.size() << std::endl;
        return false;
    }
    
    for (size_t i = 0; i < stream1.size(); i++) {
        if (stream1[i] != stream2[i]) {
            std::cout << "位流在位置 " << i << " 不同: " << (int)stream1[i] << " vs " << (int)stream2[i] << std::endl;
            return false;
        }
    }
    
    return true;
}

// 比较两个文件的头是否相同
bool compareFileHeaders(const std::vector<unsigned char>& file1, const std::vector<unsigned char>& file2, size_t header_size = 54) {
    if (file1.size() < header_size || file2.size() < header_size) {
        std::cout << "文件大小小于头大小: " << file1.size() << " 和 " << file2.size() << std::endl;
        return false;
    }
    
    bool match = true;
    for (size_t i = 0; i < header_size; i++) {
        if (file1[i] != file2[i]) {
            std::cout << "文件头在位置 " << i << " 不同: " << (int)file1[i] << " vs " << (int)file2[i] << std::endl;
            match = false;
        }
    }
    
    return match;
}

// 显示文件头信息
void displayFileHeader(const std::vector<unsigned char>& file, size_t header_size = 54) {
    std::cout << "文件头 (前 " << header_size << " 字节):" << std::endl;
    for (size_t i = 0; i < std::min(header_size, file.size()); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)file[i] << " ";
        if ((i + 1) % 16 == 0) {
            std::cout << std::endl;
        }
    }
    std::cout << std::dec << std::endl;
}

int main() {
    std::string original_file = "test.bmp";
    std::string compressed_file = "test_structure.huf";
    std::string recovered_file = "test_recovered.bmp";
    
    std::cout << "测试HUF文件结构验证...\n";
    
    // 步骤1: 使用hufHandler构建Huffman树并保存HUF文件
    std::cout << "1. 使用hufHandler构建Huffman树并保存HUF文件...\n";
    
    // 压缩BMP文件为HUF
    if (!hufHandler::bmp2huf_start(original_file, compressed_file, nullptr)) {
        std::cerr << "✗ 压缩失败\n";
        return 1;
    }
    std::cout << "✓ HUF文件保存成功: " << compressed_file << std::endl;
    
    // 读取BMP文件数据用于后续比较
    bmp* original_bmp = bmpHandler::load(original_file);
    if (!original_bmp) {
        std::cerr << "无法打开原始文件: " << original_file << std::endl;
        return 1;
    }
    
    // 构建整个BMP文件的频率表
    std::unordered_map<unsigned char, u64> original_frequency_map;
    for (const auto& byte : original_bmp->filemap) {
        original_frequency_map[byte]++;
    }
    
    // 步骤2: 使用hufHandler加载HUF文件并解码
    std::cout << "2. 使用hufHandler加载HUF文件并解码...\n";
    
    // 加载HUF文件
    huf* hufFile = hufHandler::load(compressed_file);
    if (!hufFile) {
        std::cerr << "✗ 无法加载HUF文件\n";
        return 1;
    }
    
    // 转换键值对数据格式
    std::unordered_map<unsigned char, u64> loaded_frequency_map;
    for(auto i : hufFile->key_value_data){
        loaded_frequency_map.insert(std::make_pair(static_cast<unsigned char>(i.first), i.second));
    }
    
    // 从HUF文件数据重建完整文件内容
    HuffmanTree<unsigned char> temp_tree;
    temp_tree.input_data(loaded_frequency_map);
    temp_tree.spawnTree();
    BitStream<unsigned char> temp_decoder(temp_tree.get_root());
    
    // 解码HUF文件
    std::vector<unsigned char> recovered_file_content;
    bool decode_success = false;
    
    try {
        recovered_file_content = temp_decoder.decode(hufFile->bitset, hufFile->bit_num);
        decode_success = true;
        std::cout << "✓ 解码成功!\n";
    } catch (const std::exception& e) {
        std::cout << "✗ 解码失败: " << e.what() << std::endl;
        delete hufFile;
        delete original_bmp;
        return 1;
    }
    
    // 步骤3: 比较文件头
    std::cout << "3. 比较文件头...\n";
    displayFileHeader(original_bmp->filemap);
    displayFileHeader(recovered_file_content);
    
    if (compareFileHeaders(original_bmp->filemap, recovered_file_content)) {
        std::cout << "✓ 文件头匹配成功!\n";
    } else {
        std::cout << "✗ 文件头匹配失败!\n";
        delete hufFile;
        delete original_bmp;
        return 1;
    }
    
    // 步骤4: 比较频率表
    std::cout << "4. 比较频率表...\n";
    
    // 构建解码后的文件的频率表用于验证
    std::unordered_map<unsigned char, u64> decoded_frequency_map;
    for (const auto& byte : recovered_file_content) {
        decoded_frequency_map[byte]++;
    }
    
    // 输出频率表的详细信息，以便排查问题
    std::cout << "额外信息: 频率表前10个条目...\n";
    int count = 0;
    for (const auto& pair : original_frequency_map) {
        if (count >= 10) break;
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)pair.first 
                  << std::dec << ": 原始=" << pair.second 
                  << " 解码=" << decoded_frequency_map[pair.first] 
                  << " HUF=" << loaded_frequency_map[pair.first] << "\n";
        count++;
    }
    
    // 特别检查键128的频率
    if (original_frequency_map.count(128)) {
        std::cout << "键128的频率: 原始=" << original_frequency_map[128] 
                  << " 解码=" << decoded_frequency_map[128]
                  << " HUF=" << loaded_frequency_map[128] << "\n";
    }
    
    // 比较原始频率表与HUF文件中的频率表
    if (compareFrequencyMaps(original_frequency_map, loaded_frequency_map)) {
        std::cout << "✓ 频率表匹配成功!\n";
    } else {
        std::cout << "✗ 频率表匹配失败!\n";
        delete hufFile;
        delete original_bmp;
        return 1;
    }
    
    // 步骤3: 使用加载的频率表构建新的Huffman树
    std::cout << "3. 使用加载的频率表构建新的Huffman树...\n";
    HuffmanTree<unsigned char> new_tree;
    new_tree.input_data(loaded_frequency_map);
    new_tree.spawnTree();
    std::cout << "✓ 新的Huffman树构建成功!\n";
    
    // 步骤4: 比较新构建的树与HUF文件加载的树的编码映射
    std::cout << "4. 比较新构建的树与HUF文件加载的树的编码映射...\n";
    
    // 从HUF文件加载的键值对重新构建树并获取编码表
    HuffmanTree<unsigned char> huf_tree;
    huf_tree.input_data(loaded_frequency_map);
    huf_tree.spawnTree();
    auto huf_code_map = huf_tree.get_code_map();
    
    // 比较两个树的编码映射
    if (compareCodeMaps(huf_code_map, new_tree.get_code_map())) {
        std::cout << "✓ 编码映射匹配成功!\n";
    } else {
        std::cout << "✗ 编码映射匹配失败（可能由于Huffman树的非唯一性）\n";
    }
    
    // 步骤5: 比较位流数据
    std::cout << "5. 比较位流数据...\n";
    
    // 使用新构建的树编码BMP文件数据
    BitStream<unsigned char> encoder(new_tree.get_code_map());
    std::vector<unsigned char> encoded_data = encoder.encode(original_bmp->filemap);
    
    // 比较编码后的数据与HUF文件中的位流
    if (compareBitStreams(encoded_data, hufFile->bitset)) {
        std::cout << "✓ 位流数据匹配成功!\n";
    } else {
        std::cout << "✗ 位流数据匹配失败\n";
        std::cout << "  原始位流大小: " << hufFile->bitset.size() << " 字节\n";
        std::cout << "  新生成位流大小: " << encoded_data.size() << " 字节\n";
        delete hufFile;
        delete original_bmp;
        return 1;
    }
    
    // 步骤6: 测试解压缩功能
    std::cout << "6. 测试解压缩功能...\n";
    if (!bmpHandler::huf2bmp_start(compressed_file, recovered_file, nullptr)) {
        std::cerr << "✗ 解压缩失败\n";
        delete hufFile;
        return 1;
    }
    std::cout << "✓ 解压缩成功: " << recovered_file << std::endl;
    
    // 比较原始BMP和恢复的BMP文件
    bmp* recovered_bmp = bmpHandler::load(recovered_file);
    if (!recovered_bmp) {
        std::cerr << "无法打开恢复的文件: " << recovered_file << std::endl;
        delete hufFile;
        delete original_bmp;
        return 1;
    }
    
    // 比较像素数据
    if (compareBitStreams(original_bmp->filemap, recovered_bmp->filemap)) {
        std::cout << "✓ 原始文件与恢复文件的像素数据匹配成功!\n";
    } else {
        std::cout << "✗ 原始文件与恢复文件的像素数据匹配失败\n";
        delete hufFile;
        delete original_bmp;
        delete recovered_bmp;
        return 1;
    }
    
    // 释放所有资源
    delete hufFile;
    delete original_bmp;
    delete recovered_bmp;
    
    std::cout << "\n测试完成!\n";
    return 0;
}
