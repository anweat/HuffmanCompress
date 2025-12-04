#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "../task/bmpHandler.h"
#include "../task/hufHandler.h"
#include "../huffman/huffmantree.h"
#include "../huffman/bitstream.h"
#include "../logger/Logger.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// 比较两个频率表（uint8_t -> uint64_t）是否相等
bool compareFrequencyMaps(const std::unordered_map<uint8_t, uint64_t>& map1, 
                        const std::unordered_map<uint8_t, uint64_t>& map2, 
                        std::ofstream& diff_file, 
                        const std::string& map_name) {
    diff_file << "\n=== " << map_name << " 比较 ===\n";
    
    if (map1.size() != map2.size()) {
        diff_file << "大小不同: map1=" << map1.size() << ", map2=" << map2.size() << "\n";
        return false;
    }
    
    bool all_match = true;
    
    // 检查map1中的所有键是否在map2中存在且值相等
    for (const auto& pair : map1) {
        auto it = map2.find(pair.first);
        if (it == map2.end()) {
            diff_file << "键不存在于map2: " << static_cast<int>(pair.first) << "\n";
            all_match = false;
        } else if (it->second != pair.second) {
            diff_file << "值不匹配 - 键: " << static_cast<int>(pair.first) 
                      << " | map1: " << pair.second 
                      << " | map2: " << it->second << "\n";
            all_match = false;
        }
    }
    
    // 检查map2中的所有键是否在map1中存在
    for (const auto& pair : map2) {
        auto it = map1.find(pair.first);
        if (it == map1.end()) {
            diff_file << "键不存在于map1: " << static_cast<int>(pair.first) << "\n";
            all_match = false;
        }
    }
    
    if (all_match) {
        diff_file << "所有键值对匹配\n";
    }
    
    return all_match;
}

// 输出频率表到文件
void printFrequencyMapToFile(const std::unordered_map<uint8_t, uint64_t>& map, 
                            std::ofstream& file, 
                            const std::string& map_name) {
    file << "\n=== " << map_name << " ===\n";
    for (const auto& pair : map) {
        file << "键: " << static_cast<int>(pair.first) << " | 频率: " << pair.second << "\n";
    }
}

// 为std::pair<uint64_t, uint8_t>类型添加输出辅助函数
void printPairToFile(const std::pair<uint64_t, uint8_t>& pair, std::ofstream& file) {
    file << "(频率: " << pair.first << ", 长度: " << static_cast<int>(pair.second) << ")";
}

// 比较两个编码表（uint8_t -> pair<uint64_t, uint8_t>）是否相等
bool compareCodeMaps(const std::unordered_map<uint8_t, std::pair<uint64_t, uint8_t>>& map1, 
                    const std::unordered_map<uint8_t, std::pair<uint64_t, uint8_t>>& map2, 
                    std::ofstream& diff_file, 
                    const std::string& map_name) {
    diff_file << "\n=== " << map_name << " 比较 ===\n";
    
    if (map1.size() != map2.size()) {
        diff_file << "大小不同: map1=" << map1.size() << ", map2=" << map2.size() << "\n";
        return false;
    }
    
    bool all_match = true;
    
    // 检查map1中的所有键是否在map2中存在且值相等
    for (const auto& pair : map1) {
        auto it = map2.find(pair.first);
        if (it == map2.end()) {
            diff_file << "键不存在于map2: " << static_cast<int>(pair.first) << "\n";
            all_match = false;
        } else if (it->second != pair.second) {
            diff_file << "值不匹配 - 键: " << static_cast<int>(pair.first) << " | map1: ";
            printPairToFile(pair.second, diff_file);
            diff_file << " | map2: ";
            printPairToFile(it->second, diff_file);
            diff_file << "\n";
            all_match = false;
        }
    }
    
    // 检查map2中的所有键是否在map1中存在
    for (const auto& pair : map2) {
        auto it = map1.find(pair.first);
        if (it == map1.end()) {
            diff_file << "键不存在于map1: " << static_cast<int>(pair.first) << "\n";
            all_match = false;
        }
    }
    
    if (all_match) {
        diff_file << "所有键值对匹配\n";
    }
    
    return all_match;
}

// 比较两个vector是否相等
template<typename T>
bool compareVectors(const std::vector<T>& vec1, const std::vector<T>& vec2, std::ofstream& diff_file, const std::string& vec_name) {
    diff_file << "\n=== " << vec_name << " 比较 ===\n";
    
    if (vec1.size() != vec2.size()) {
        diff_file << "大小不同: vec1=" << vec1.size() << ", vec2=" << vec2.size() << "\n";
        return false;
    }
    
    // 比较前100个不匹配的元素，避免文件过大
    int max_diffs = 100;
    int diff_count = 0;
    bool all_match = true;
    
    for (size_t i = 0; i < vec1.size(); i++) {
        if (vec1[i] != vec2[i]) {
            diff_file << "位置 " << i << ": vec1=0x" << std::hex << static_cast<int>(vec1[i]) 
                      << " | vec2=0x" << std::hex << static_cast<int>(vec2[i]) << std::dec << "\n";
            all_match = false;
            diff_count++;
            if (diff_count >= max_diffs) {
                diff_file << "... 还有更多不匹配，已截断显示\n";
                break;
            }
        }
    }
    
    if (all_match) {
        diff_file << "所有元素匹配\n";
    }
    
    return all_match;
}

// 打印比较结果
void printComparisonResult(const std::string& testName, bool result) {
    std::cout << "[" << (result ? "PASS" : "FAIL") << "] " << testName << std::endl;
}

int main() {
    // 启用日志
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    
    // 测试文件路径
    std::string input_bmp = "D:\\Desktop\\bmp\\test1.bmp";
    std::string output_huf = "D:\\Desktop\\bmp\\test1_output.huf";
    std::string diff_file_path = "compare_results.txt";
    
    std::cout << "=======================================" << std::endl;
    std::cout << "两条处理路径比较测试" << std::endl;
    std::cout << "输入文件: " << input_bmp << std::endl;
    std::cout << "临时HUF文件: " << output_huf << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // 检查输入文件是否存在
    std::ifstream bmp_file(input_bmp, std::ios::binary);
    if (!bmp_file) {
        std::cerr << "错误: 找不到输入文件 " << input_bmp << std::endl;
        return 1;
    }
    bmp_file.close();
    
    // =======================================================================
    // 路径二: 手动实现频率分析、编码和解码
    // =======================================================================
    std::cout << "\n路径二: 手动实现频率分析、编码和解码" << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    
    // 路径二的结果数据
    bmp* bmp_file_path2 = nullptr;
    std::unordered_map<u8, u64> freq_map_path2;
    HuffmanTree<u8> tree_path2;
    std::vector<u8> encoded_data_path2;
    std::vector<u8> decoded_data_path2;
    bool path2_success = false;
    bool path2_decode_success = false; // 解码是否成功的标志
    
    try {
        // 步骤1: 加载BMP文件
        std::cout << "1. 加载BMP文件..." << std::endl;
        bmp_file_path2 = bmpHandler::load(input_bmp);
        if (!bmp_file_path2) {
            std::cerr << "错误: 加载BMP文件失败" << std::endl;
            return 1;
        }
        std::cout << "   ✓ BMP文件加载成功" << std::endl;
        std::cout << "   - BMP文件大小: " << bmp_file_path2->bit_num << " 字节" << std::endl;
        
        // 步骤2: 手动计算频率
        std::cout << "2. 手动计算频率..." << std::endl;
        for (u8 byte : bmp_file_path2->filemap) {
            freq_map_path2[byte]++;
        }
        std::cout << "   ✓ 频率计算成功" << std::endl;
        std::cout << "   - 手动计算的频率表大小: " << freq_map_path2.size() << std::endl;
        
        // 步骤3: 手动建立Huffman树
        std::cout << "3. 手动建立Huffman树..." << std::endl;
        tree_path2.input_data(freq_map_path2);
        tree_path2.spawnTree();
        std::cout << "   ✓ Huffman树建立成功" << std::endl;
        
        // 步骤4: 手动编码位集
        std::cout << "4. 手动编码位集..." << std::endl;
        BitStream<u8> encoder_path2(tree_path2.get_code_map());
        encoded_data_path2 = encoder_path2.encode(bmp_file_path2->filemap);
        std::cout << "   ✓ 位集编码成功" << std::endl;
        std::cout << "   - 编码后的数据长度: " << encoded_data_path2.size() << " 字节" << std::endl;
        
        // 步骤5: 手动解码位集
        std::cout << "5. 手动解码位集..." << std::endl;
        try {
            BitStream<u8> decoder_path2(tree_path2.get_root());
            decoded_data_path2 = decoder_path2.decode(encoded_data_path2, bmp_file_path2->bit_num);
            std::cout << "   ✓ 位集解码成功" << std::endl;
            std::cout << "   - 解码后的数据长度: " << decoded_data_path2.size() << " 字节" << std::endl;
            std::cout << "   - 原始BMP数据长度: " << bmp_file_path2->bit_num << " 字节" << std::endl;
            path2_decode_success = true;
        } catch (const std::exception& e) {
            std::cerr << "   ✗ 位集解码失败: " << e.what() << std::endl;
            path2_decode_success = false;
            // 解码失败不影响其他比较，继续执行
        }
        
        path2_success = true;
        
    } catch (const std::exception& e) {
        std::cerr << "路径二执行出错: " << e.what() << std::endl;
        path2_success = false;
    }
    
    // =======================================================================
    // 路径一: 使用现有bmp2huf功能和HUF文件解码
    // =======================================================================
    std::cout << "\n路径一: 使用现有bmp2huf功能和HUF文件解码" << std::endl;
    std::cout << "------------------------------------------------------" << std::endl;
    
    // 路径一的结果数据
    huf* huf_file = nullptr;
    std::unordered_map<u8, u64> freq_map_path1;
    HuffmanTree<u8> tree_path1;
    std::vector<u8> decoded_data_path1;
    bool path1_success = false;
    bool path1_decode_success = false; // 解码是否成功的标志
    std::string path1_error = "";
    
    try {
        // 步骤1: BMP转HUF
        double process = 0.0;
        std::cout << "1. 执行BMP转HUF..." << std::endl;
        bool success = hufHandler::bmp2huf_start(input_bmp, output_huf, &process);
        if (!success) {
            std::cerr << "错误: BMP转HUF失败" << std::endl;
            path1_error = "BMP转HUF失败";
            throw std::runtime_error(path1_error);
        }
        std::cout << "   ✓ BMP转HUF成功" << std::endl;
        
        // 步骤2: 读取HUF文件
        std::cout << "2. 读取HUF文件..." << std::endl;
        huf_file = hufHandler::load(output_huf);
        if (!huf_file) {
            std::cerr << "错误: 读取HUF文件失败" << std::endl;
            path1_error = "读取HUF文件失败";
            throw std::runtime_error(path1_error);
        }
        std::cout << "   ✓ HUF文件读取成功" << std::endl;
        std::cout << "   - HUF文件头信息: key_num=" << huf_file->key_num << ", bit_num=" << huf_file->bit_num << std::endl;
        
        // 步骤3: 从HUF文件重建Huffman树
        std::cout << "3. 从HUF文件重建Huffman树..." << std::endl;
        // 转换键值对格式
        for (const auto& pair : huf_file->key_value_data) {
            freq_map_path1[static_cast<u8>(pair.first)] = pair.second;
        }
        tree_path1.input_data(freq_map_path1);
        tree_path1.spawnTree();
        std::cout << "   ✓ Huffman树重建成功" << std::endl;
        std::cout << "   - 从HUF文件获取的频率表大小: " << freq_map_path1.size() << std::endl;
        
        // 步骤4: 解码HUF位集
        std::cout << "4. 解码HUF位集..." << std::endl;
        try {
            BitStream<u8> decoder_path1(tree_path1.get_root());
            decoded_data_path1 = decoder_path1.decode(huf_file->bitset, huf_file->bit_num);
            std::cout << "   ✓ 位集解码成功" << std::endl;
            std::cout << "   - 解码后的数据长度: " << decoded_data_path1.size() << std::endl;
            path1_decode_success = true;
        } catch (const std::exception& e) {
            std::cerr << "   ✗ 位集解码失败: " << e.what() << std::endl;
            path1_decode_success = false;
            // 解码失败不影响其他比较，继续执行
        }
        
        path1_success = true;
        
    } catch (const std::exception& e) {
        std::cerr << "路径一执行出错: " << e.what() << std::endl;
        path1_success = false;
        path1_error = e.what();
    }
    
    // 创建差异文件
    std::ofstream diff_file(diff_file_path);
    if (!diff_file.is_open()) {
        std::cerr << "错误: 无法创建差异文件 " << diff_file_path << std::endl;
        return 1;
    }
    
    // 记录基本信息
    diff_file << "=======================================\n";
    diff_file << "两条处理路径比较测试结果\n";
    diff_file << "=======================================\n";
    diff_file << "输入文件: " << input_bmp << "\n";
    diff_file << "临时HUF文件: " << output_huf << "\n";
    diff_file << "生成时间: " << __DATE__ << " " << __TIME__ << "\n";
    diff_file << "路径一执行状态: " << (path1_success ? "成功" : "失败") << "\n";
    if (!path1_success) {
        diff_file << "路径一错误信息: " << path1_error << "\n";
    }
    diff_file << "路径一解码状态: " << (path1_decode_success ? "成功" : "失败") << "\n";
    diff_file << "路径二执行状态: " << (path2_success ? "成功" : "失败") << "\n";
    diff_file << "路径二解码状态: " << (path2_decode_success ? "成功" : "失败") << "\n";
    
    // =======================================================================
    // 比较两条路径的结果
    // =======================================================================
    std::cout << "\n=======================================" << std::endl;
    std::cout << "比较两条路径的结果" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    bool all_tests_pass = true;
    
    // 比较1: 文件头信息
    std::cout << "\n1. 比较文件头信息: " << std::flush;
    bool header_match = false;
    if (path1_success && path2_success && huf_file && bmp_file_path2) {
        header_match = (huf_file->key_num == tree_path2.get_frequency_map().size()) && 
                       (huf_file->bit_num == bmp_file_path2->bit_num) &&
                       (huf_file->key_size == 1); // 对于u8类型，key_size应该是1
    }
    printComparisonResult("文件头信息", header_match);
    all_tests_pass &= header_match;
    
    diff_file << "\n=======================================\n";
    diff_file << "文件头信息比较\n";
    diff_file << "=======================================\n";
    if (huf_file) {
        diff_file << "HUF文件头: key_num=" << huf_file->key_num << ", bit_num=" << huf_file->bit_num << ", key_size=" << huf_file->key_size << ", value_size=" << huf_file->value_size << "\n";
    } else {
        diff_file << "HUF文件头: 不可用 (路径一执行失败)\n";
    }
    if (bmp_file_path2) {
        diff_file << "手动计算: unique_chars=" << tree_path2.get_frequency_map().size() << ", bit_num=" << bmp_file_path2->bit_num << "\n";
    } else {
        diff_file << "手动计算: 不可用 (路径二执行失败)\n";
    }
    diff_file << "比较结果: " << (header_match ? "匹配" : "不匹配") << "\n";
    
    // 比较2: 频率表
    std::cout << "2. 比较频率表: " << std::flush;
    bool freq_match = false;
    if (path1_success && path2_success) {
        freq_match = compareFrequencyMaps(freq_map_path1, freq_map_path2, diff_file, "频率表");
    }
    printComparisonResult("频率表", freq_match);
    all_tests_pass &= freq_match;
    
    // 将频率表详细内容输出到文件
    if (path1_success) {
        printFrequencyMapToFile(freq_map_path1, diff_file, "路径一频率表");
    }
    if (path2_success) {
        printFrequencyMapToFile(freq_map_path2, diff_file, "路径二频率表");
    }
    
    // 比较3: 编码表
    std::cout << "3. 比较编码表: " << std::flush;
    bool code_match = false;
    if (path1_success && path2_success) {
        code_match = compareCodeMaps(tree_path1.get_code_map(), tree_path2.get_code_map(), diff_file, "编码表");
    }
    printComparisonResult("编码表", code_match);
    all_tests_pass &= code_match;
    
    // 比较4: 位集
    std::cout << "4. 比较位集: " << std::flush;
    bool bitset_match = false;
    if (path1_success && path2_success && huf_file) {
        bitset_match = compareVectors(huf_file->bitset, encoded_data_path2, diff_file, "位集");
    }
    printComparisonResult("位集", bitset_match);
    all_tests_pass &= bitset_match;
    
    // 比较5: 解码后的数据
    std::cout << "5. 比较解码后的数据: " << std::flush;
    bool decoded_data_match = false;
    if (path1_success && path2_success && path1_decode_success && path2_decode_success) {
        decoded_data_match = compareVectors(decoded_data_path1, decoded_data_path2, diff_file, "解码后的数据");
    }
    printComparisonResult("解码后的数据", decoded_data_match);
    all_tests_pass &= decoded_data_match;
    
    // 比较6: 解码后的数据与原始数据
    std::cout << "6. 比较解码后的数据与原始数据: " << std::flush;
    bool original_match = false;
    if (path1_success && path2_success && path1_decode_success && bmp_file_path2) {
        original_match = compareVectors(decoded_data_path1, bmp_file_path2->filemap, diff_file, "解码数据与原始数据");
    }
    printComparisonResult("解码后的数据与原始数据", original_match);
    all_tests_pass &= original_match;
    
    // 关闭差异文件
    diff_file.close();
    std::cout << "\n详细比较结果已输出到: " << diff_file_path << std::endl;
    
    // =======================================================================
    // 清理资源
    // =======================================================================
    std::cout << "\n=======================================" << std::endl;
    std::cout << "清理资源" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    // 释放内存
    if (huf_file) {
        delete huf_file;
    }
    if (bmp_file_path2) {
        delete bmp_file_path2;
    }
    
    // 删除临时HUF文件
    if (std::remove(output_huf.c_str()) != 0) {
        std::cout << "警告: 无法删除临时HUF文件" << std::endl;
    } else {
        std::cout << "✓ 临时HUF文件已删除" << std::endl;
    }
    
    // =======================================================================
    // 测试结果总结
    // =======================================================================
    std::cout << "\n=======================================" << std::endl;
    std::cout << "测试结果总结" << std::endl;
    std::cout << "=======================================" << std::endl;
    
    std::cout << "路径一执行状态: " << (path1_success ? "✅ 成功" : "❌ 失败") << std::endl;
    std::cout << "路径一解码状态: " << (path1_decode_success ? "✅ 成功" : "❌ 失败") << std::endl;
    if (!path1_success) {
        std::cout << "路径一错误信息: " << path1_error << std::endl;
    }
    std::cout << "路径二执行状态: " << (path2_success ? "✅ 成功" : "❌ 失败") << std::endl;
    std::cout << "路径二解码状态: " << (path2_decode_success ? "✅ 成功" : "❌ 失败") << std::endl;
    
    if (path1_success && path2_success) {
        if (all_tests_pass) {
            std::cout << "✅ 所有测试通过！两条处理路径结果完全一致。" << std::endl;
        } else {
            std::cout << "❌ 部分测试失败！请查看 " << diff_file_path << " 获取详细差异信息。" << std::endl;
        }
    } else {
        std::cout << "⚠️  由于一条或两条路径执行失败，无法完成完整比较。" << std::endl;
    }
    
    std::cout << "\n测试完成！" << std::endl;
    
    return all_tests_pass ? 0 : 1;
}