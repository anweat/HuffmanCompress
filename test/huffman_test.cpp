#include "huffman/huffmantree.h"
#include "FileStream/FileReader.h"
#include "FileStream/File.h"
#include <iostream>
#include <string>
#include <map>

// 辅助函数：打印Huffman树的节点信息
void printHuffmanTreeInfo(const HuffmanTree<unsigned char>& tree) {
    std::cout << "=== Huffman Tree Information ===" << std::endl;
    
    // Get and print first 20 characters' Huffman codes (avoid too much output)
    std::cout << "\nFirst 20 characters' Huffman codes: " << std::endl;
    const auto& code_map = tree.get_code_map();
    int count = 0;
    for (const auto& pair : code_map) {
        if (count >= 20) break;
        
        unsigned char ch = pair.first;
        const auto& code_info = pair.second;
        
        // Get frequency from frequency_map
        const auto& frequency_map = tree.get_frequency_map();
        auto freq_iter = frequency_map.find(ch);
        u64 frequency = (freq_iter != frequency_map.end()) ? freq_iter->second : 0;
        
        std::cout << "Char: 0x" << std::hex << static_cast<int>(ch) << " (" << 
            static_cast<int>(ch) << ") | Frequency: " << frequency << 
            " | Code: ";
        
        // Print binary code
        u64 code = code_info.first;
        u8 code_length = code_info.second;
        
        for (int i = 0; i < code_length; i++) {
            bool bit = (code >> (code_length - 1 - i)) & 1;
            std::cout << (bit ? "1" : "0");
        }
        
        std::cout << " | Code length: " << static_cast<int>(code_length) << std::endl;
        count++;
    }
    
    if (code_map.size() > 20) {
        std::cout << "... " << code_map.size() - 20 << " more character codes not shown" << std::endl;
    }
    
    std::cout << "Total number of unique characters: " << code_map.size() << std::endl;
}

int main() {
    // Set test file path
    std::string bmpFilePath = "D:\\Desktop\\bmp\\test1.bmp";
    
    std::cout << "=== Huffman Tree Generation Test ===" << std::endl;
    std::cout << "Test file: " << bmpFilePath << std::endl;
    
    try {
        // 1. Read BMP file data
        std::cout << "\n1. Reading BMP file data..." << std::endl;
        
        // Create file reader
        FileReader fileReader(bmpFilePath);
        
        // Get file size
        int64_t fileSize = fileReader.getFile().getFileSize();
        std::cout << "File size: " << fileSize << " bytes" << std::endl;
        
        // Read file data (we only read part of the data for testing Huffman tree generation)
        const int BUFFER_SIZE = 1024 * 1024; // 1MB buffer
        unsigned char* buffer = new unsigned char[BUFFER_SIZE];
        
        // Read data directly using input stream
        std::ifstream& inputStream = fileReader.getFile().getInputStream();
        inputStream.read(reinterpret_cast<char*>(buffer), BUFFER_SIZE);
        int64_t bytesRead = inputStream.gcount();
        std::cout << "Actual read: " << bytesRead << " bytes" << std::endl;
        
        // 2. Create and build Huffman tree
        std::cout << "\n2. Creating and building Huffman tree..." << std::endl;
        
        HuffmanTree<unsigned char> huffmanTree;
        
        // Count character frequencies
        std::cout << "   Counting character frequencies..." << std::endl;
        for (int64_t i = 0; i < bytesRead; i++) {
            huffmanTree.input_data(buffer[i]);
        }
        
        // Generate Huffman tree
        std::cout << "   Generating Huffman tree..." << std::endl;
        if (!huffmanTree.spawnTree()) {
            std::cerr << "Error: Failed to generate Huffman tree!" << std::endl;
            delete[] buffer;
            return 1;
        }
        
        std::cout << "   Huffman tree generated successfully!" << std::endl;
        
        // 3. Print Huffman tree information
        printHuffmanTreeInfo(huffmanTree);
        
        // 4. Verify Huffman codes
        std::cout << "\n3. Verifying Huffman codes..." << std::endl;
        
        // Check if codes are unique and prefix-free
        const auto& code_map = huffmanTree.get_code_map();
        bool has_error = false;
        
        for (const auto& pair1 : code_map) {
            for (const auto& pair2 : code_map) {
                if (&pair1 == &pair2) continue;
                
                u8 ch1 = pair1.first;
                u8 ch2 = pair2.first;
                
                const auto& code_info1 = pair1.second;
                const auto& code_info2 = pair2.second;
                
                u64 code1 = code_info1.first;
                u64 code2 = code_info2.first;
                u8 len1 = code_info1.second;
                u8 len2 = code_info2.second;
                
                // Check for prefix conflicts between codes
                int min_len = std::min(len1, len2);
                
                // Compare the first min_len bits
                bool prefix_match = true;
                for (int i = 0; i < min_len; i++) {
                    bool bit1 = (code1 >> (len1 - 1 - i)) & 1;
                    bool bit2 = (code2 >> (len2 - 1 - i)) & 1;
                    
                    if (bit1 != bit2) {
                        prefix_match = false;
                        break;
                    }
                }
                
                if (prefix_match) {
                    std::cerr << "Error: Code prefix conflict detected!" << std::endl;
                    
                    std::cerr << "Character 1: 0x" << std::hex << static_cast<int>(ch1) << ", Code: ";
                    for (int i = 0; i < len1; i++) {
                        bool bit = (code1 >> (len1 - 1 - i)) & 1;
                        std::cerr << (bit ? "1" : "0");
                    }
                    std::cerr << std::endl;
                    
                    std::cerr << "Character 2: 0x" << std::hex << static_cast<int>(ch2) << ", Code: ";
                    for (int i = 0; i < len2; i++) {
                        bool bit = (code2 >> (len2 - 1 - i)) & 1;
                        std::cerr << (bit ? "1" : "0");
                    }
                    std::cerr << std::endl;
                    
                    has_error = true;
                    break;
                }
            }
            
            if (has_error) break;
        }
        
        if (!has_error) {
            std::cout << "   Huffman code verification passed: All codes are unique and prefix-free!" << std::endl;
        }
        
        // 5. Calculate average code length
        std::cout << "\n4. Calculating average code length..." << std::endl;
        
        double total_bits = 0;
        u64 total_frequency = 0;
        
        const auto& frequency_map = huffmanTree.get_frequency_map();
        
        for (const auto& pair : frequency_map) {
            u8 ch = pair.first;
            u64 frequency = pair.second;
            
            // Get corresponding code length
            auto code_iter = code_map.find(ch);
            if (code_iter != code_map.end()) {
                u8 code_length = code_iter->second.second;
                total_bits += frequency * code_length;
                total_frequency += frequency;
            }
        }
        
        if (total_frequency > 0) {
            double average_length = total_bits / total_frequency;
            std::cout << "   Average code length: " << average_length << " bits/character" << std::endl;
        }
        
        // Clean up resources
        delete[] buffer;
        
        if (has_error) {
            return 1;
        }
        
        std::cout << "\n=== Huffman Tree Test Completed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
