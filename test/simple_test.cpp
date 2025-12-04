#include "./huffman/huffmantree.h"
#include <iostream>
#include <unordered_map>

// A simple test program to verify Huffman tree generation
int main() {
    std::cout << "=== Simple Huffman Tree Test ===" << std::endl;
    
    // Create a simple test dataset with known frequencies
    std::unordered_map<unsigned char, u64> test_data = {
        {static_cast<unsigned char>('a'), 10},
        {static_cast<unsigned char>('b'), 5},
        {static_cast<unsigned char>('c'), 2},
        {static_cast<unsigned char>('d'), 1}
    };
    
    // Create Huffman tree
    HuffmanTree<unsigned char> huffmanTree;
    
    // Input data
    huffmanTree.input_data(test_data);
    
    // Generate Huffman tree
    if (!huffmanTree.spawnTree()) {
        std::cerr << "Error: Failed to generate Huffman tree!" << std::endl;
        return 1;
    }
    
    std::cout << "Huffman tree generated successfully!" << std::endl;
    
    // Get and display codes
    std::cout << "\nHuffman Codes:" << std::endl;
    const auto& code_map = huffmanTree.get_code_map();
    
    for (const auto& pair : code_map) {
        unsigned char ch = pair.first;
        const auto& code_info = pair.second;
        u64 code = code_info.first;
        u8 code_length = code_info.second;
        
        std::cout << "Character: '" << static_cast<char>(ch) << "' | Frequency: " << test_data[ch] << " | Code: ";
        
        // Print binary code
        for (int i = 0; i < code_length; i++) {
            bool bit = (code >> (code_length - 1 - i)) & 1;
            std::cout << (bit ? "1" : "0");
        }
        
        std::cout << " | Length: " << static_cast<int>(code_length) << std::endl;
    }
    
    // Verify that codes are prefix-free
    std::cout << "\nVerifying prefix-free codes..." << std::endl;
    bool has_prefix_conflict = false;
    
    for (const auto& pair1 : code_map) {
        for (const auto& pair2 : code_map) {
            if (&pair1 == &pair2) continue;
            
            const auto& code_info1 = pair1.second;
            const auto& code_info2 = pair2.second;
            
            u64 code1 = code_info1.first;
            u64 code2 = code_info2.first;
            u8 len1 = code_info1.second;
            u8 len2 = code_info2.second;
            
            // Check if one code is a prefix of the other
            bool is_prefix = true;
            int min_len = std::min(len1, len2);
            
            for (int i = 0; i < min_len; i++) {
                bool bit1 = (code1 >> (len1 - 1 - i)) & 1;
                bool bit2 = (code2 >> (len2 - 1 - i)) & 1;
                
                if (bit1 != bit2) {
                    is_prefix = false;
                    break;
                }
            }
            
            if (is_prefix) {
                std::cerr << "Error: Prefix conflict between codes!" << std::endl;
                has_prefix_conflict = true;
                break;
            }
        }
        
        if (has_prefix_conflict) break;
    }
    
    if (!has_prefix_conflict) {
        std::cout << "All codes are prefix-free!" << std::endl;
    }
    
    // Calculate average code length
    double total_bits = 0;
    u64 total_frequency = 0;
    
    for (const auto& pair : test_data) {
        unsigned char ch = pair.first;
        u64 frequency = pair.second;
        
        auto code_iter = code_map.find(ch);
        if (code_iter != code_map.end()) {
            u8 code_length = code_iter->second.second;
            total_bits += frequency * code_length;
            total_frequency += frequency;
        }
    }
    
    if (total_frequency > 0) {
        double average_length = total_bits / total_frequency;
        std::cout << "\nAverage code length: " << average_length << " bits/character" << std::endl;
    }
    
    std::cout << "\n=== Test Complete! ===" << std::endl;
    return 0;
}