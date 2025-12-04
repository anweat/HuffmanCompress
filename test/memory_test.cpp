#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>
// Include necessary headers
#include "task/bmpHandler.h"
#include "task/hufHandler.h"
#include "huffman/huffmantree.h"
#include "huffman/bitstream.h"


// Test configuration flags - set to true to enable comparison output for each section
const bool ENABLE_FREQUENCY_MAP_COMPARISON = true;  // Frequency map comparison output
const bool ENABLE_QUEUE_ORDER_COMPARISON = true;    // Queue order comparison output
const bool ENABLE_TREE_VALIDATION = true;            // Tree construction validation
const bool ENABLE_CODE_MAP_COMPARISON = true;        // Code map comparison output
const bool ENABLE_RECHECK_CODE_MAP_COMPARISON = false; // Rechecked code map comparison output
const bool ENABLE_FILE_COMPARISON = true;            // Original vs decompressed file comparison output



// Forward declaration for the tree validation function
template <typename T>
bool validate_huffman_tree(HuffmanTree<T>& tree, bool verbose = false);

// Function to validate Huffman tree construction
// Returns true if tree is valid, false otherwise
template <typename T>
bool validate_huffman_tree(HuffmanTree<T>& tree, bool verbose) {
    using namespace std;
    using Node = node<T>;
    
    Node* root = tree.get_root();
    if (!root) {
        if (verbose) {
            cerr << "Error: Tree root is null!" << endl;
        }
        return false;
    }
    
    bool isValid = true;
    vector<Node*> stack;
    stack.push_back(root);
    
    while (!stack.empty() && isValid) {
        Node* current = stack.back();
        stack.pop_back();
        
        // Skip validation for leaf nodes
        if (current->is_leaf) {
            // Verify leaf node has no children
            if (current->left_child || current->right_child) {
                if (verbose) {
                    cerr << "Error: Leaf node has children!" << endl;
                }
                isValid = false;
            }
            continue;
        }
        
        // For internal nodes, verify they have both children
        if (!current->left_child || !current->right_child) {
            if (verbose) {
                cerr << "Error: Internal node missing children!" << endl;
            }
            isValid = false;
            continue;
        }
        
        // Verify parent frequency is sum of children frequencies
        u64 expected_frequency = current->left_child->frequency + current->right_child->frequency;
        if (current->frequency != expected_frequency) {
            if (verbose) {
                cerr << "Error: Parent frequency mismatch! Parent: " << current->frequency 
                     << ", Expected: " << expected_frequency 
                     << " (Left: " << current->left_child->frequency 
                     << ", Right: " << current->right_child->frequency << ")" << endl;
            }
            isValid = false;
        }
        
        // Verify parent-child relationships are correctly set
        if (current->left_child->parent != current || current->right_child->parent != current) {
            if (verbose) {
                cerr << "Error: Parent-child relationship incorrect!" << endl;
            }
            isValid = false;
        }
        
        // Verify child codes are derived from parent code
        // Left child gets parent code with 0 appended
        u64 expected_left_code = current->code << 1;
        u8 expected_left_length = current->code_length + 1;
        if (current->left_child->code != expected_left_code || current->left_child->code_length != expected_left_length) {
            if (verbose) {
                cerr << "Error: Left child code mismatch! Expected (" << expected_left_code 
                     << ", " << (int)expected_left_length << "), Got (" << current->left_child->code 
                     << ", " << (int)current->left_child->code_length << ")" << endl;
            }
            isValid = false;
        }
        
        // Right child gets parent code with 1 appended
        u64 expected_right_code = (current->code << 1) | 1;
        u8 expected_right_length = current->code_length + 1;
        if (current->right_child->code != expected_right_code || current->right_child->code_length != expected_right_length) {
            if (verbose) {
                cerr << "Error: Right child code mismatch! Expected (" << expected_right_code 
                     << ", " << (int)expected_right_length << "), Got (" << current->right_child->code 
                     << ", " << (int)current->right_child->code_length << ")" << endl;
            }
            isValid = false;
        }
        
        // Push children onto stack for further validation
        stack.push_back(current->right_child);
        stack.push_back(current->left_child);
    }
    
    return isValid;
}


int main(int argc, char* argv[]) {
    
    std::string bmp_file_path = argv[1];
    
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <bmp_file_path>" << std::endl;
        return 1;
    }

    
    
    // Step 1: Load BMP file
    std::cout << "Loading BMP file: " << bmp_file_path << std::endl;
    bmp* original_bmp = bmpHandler::load(bmp_file_path);
    if (!original_bmp) {
        std::cerr << "Failed to load BMP file" << std::endl;
        return 1;
    }
    
    std::cout << "BMP file loaded successfully. Size: " << original_bmp->bit_num << " bytes" << std::endl;
    
    try {
        // Step 2: Compress BMP to HUF in memory
        std::cout << "\nCompressing BMP to HUF in memory..." << std::endl;
        
        // Create Huffman tree
        HuffmanTree<u8> tree;
        
        // Input data to Huffman tree
        for (u32 i = 0; i < original_bmp->bit_num; i++) {
            tree.input_data(original_bmp->filemap[i]);
        }
        
        // Build Huffman tree
        tree.spawnTree();
        
        // Encode bitstream
        std::vector<u8> encoded_bits = BitStream<u8>(tree.get_code_map()).encode(original_bmp->filemap);
        
        // Create HUF data structure in memory
        std::unordered_map<u64, u64> key_value_data;
        for (auto& pair : tree.get_frequency_map()) {
            key_value_data[static_cast<u64>(pair.first)] = pair.second;
        }
        
        std::cout << "Compression completed." << std::endl;
        std::cout << "Original size: " << original_bmp->bit_num << " bytes" << std::endl;
        std::cout << "Compressed size: " << encoded_bits.size() << " bytes" << std::endl;
        std::cout << "Key-value pairs: " << key_value_data.size() << std::endl;
        
        // Get original frequency map
        const auto& original_freq_map = tree.get_frequency_map();
        
        // Step 3: Decompress HUF to BMP in memory
        std::cout << "\nDecompressing HUF to BMP in memory..." << std::endl;
        
        // Create Huffman tree for decompression
        HuffmanTree<u8> decode_tree;
        
        // Convert key_value_data to correct type
        std::unordered_map<u8, u64> decode_key_value;
        for (auto& pair : key_value_data) {
            decode_key_value[static_cast<u8>(pair.first)] = pair.second;
        }
        
        // Input data to Huffman tree
        decode_tree.input_data(decode_key_value);
        
        // Build Huffman tree
        decode_tree.spawnTree();
        
        // Get decoded frequency map
        const auto& decoded_freq_map = decode_tree.get_frequency_map();
        
        // Compare frequency maps
        bool freq_map_match = true;
        
        if (original_freq_map.size() != decoded_freq_map.size()) {
            if (ENABLE_FREQUENCY_MAP_COMPARISON) {
                std::cout << "\nComparing frequency maps..." << std::endl;
                std::cerr << "Error: Frequency map size mismatch! Original: " << original_freq_map.size() << ", Decoded: " << decoded_freq_map.size() << std::endl;
            }
            freq_map_match = false;
        } else {
            for (const auto& pair : original_freq_map) {
                u8 value = pair.first;
                u64 freq = pair.second;
                
                if (decoded_freq_map.find(value) == decoded_freq_map.end()) {
                    if (ENABLE_FREQUENCY_MAP_COMPARISON) {
                        if (freq_map_match) { // Only print header once
                            std::cout << "\nComparing frequency maps..." << std::endl;
                        }
                        std::cerr << "Error: Value " << static_cast<int>(value) << " not found in decoded frequency map!" << std::endl;
                    }
                    freq_map_match = false;
                } else if (decoded_freq_map.at(value) != freq) {
                    if (ENABLE_FREQUENCY_MAP_COMPARISON) {
                        if (freq_map_match) { // Only print header once
                            std::cout << "\nComparing frequency maps..." << std::endl;
                        }
                        std::cerr << "Error: Frequency mismatch for value " << static_cast<int>(value) << "! Original: " << freq << ", Decoded: " << decoded_freq_map.at(value) << std::endl;
                    }
                    freq_map_match = false;
                }
            }
        }
        
        if (ENABLE_FREQUENCY_MAP_COMPARISON) {
            if (freq_map_match) {
                std::cout << "Success: Frequency maps match!" << std::endl;
            } else {
                std::cerr << "Error: Frequency maps do not match!" << std::endl;
            }
        }
        
        // Compare queue orders before building the tree
        bool queue_match = true;
        
        // Create temporary trees to compare queues
        HuffmanTree<u8> temp_tree;
        HuffmanTree<u8> temp_decode_tree;
        
        // Input data to temporary trees
        // Original tree queue: use the key_value_data that was actually encoded
        temp_tree.input_data(decode_key_value);  // This is the exact data that was passed through encoding
        temp_decode_tree.input_data(decoded_freq_map);  // This is the data from the decoded tree
        
        // Get copied queues from both temporary trees
        auto tree_queue = temp_tree.queue_copy();
        auto decode_tree_queue = temp_decode_tree.queue_copy();
        
        if (tree_queue.size() != decode_tree_queue.size()) {
            if (ENABLE_QUEUE_ORDER_COMPARISON) {
                std::cout << "\nComparing Huffman tree node queue orders (before tree building)..." << std::endl;
                std::cerr << "Error: Queue size mismatch! Original: " << tree_queue.size() << ", Decoded: " << decode_tree_queue.size() << std::endl;
            }
            queue_match = false;
        } else {
            // Compare the order of nodes in the queues
            while (!tree_queue.empty() && !decode_tree_queue.empty()) {
                node<u8>* original_node = tree_queue.top();
                node<u8>* decoded_node = decode_tree_queue.top();
                
                // Check if both nodes are leaves (they should be before tree building)
                if (!original_node->is_leaf || !decoded_node->is_leaf) {
                    if (ENABLE_QUEUE_ORDER_COMPARISON) {
                        if (queue_match) { // Only print header once
                            std::cout << "\nComparing Huffman tree node queue orders (before tree building)..." << std::endl;
                        }
                        std::cerr << "Error: Node is not a leaf before tree building!" << std::endl;
                    }
                    queue_match = false;
                } else {
                    // Compare leaf node data and frequency
                    if (original_node->data != decoded_node->data) {
                        if (ENABLE_QUEUE_ORDER_COMPARISON) {
                            if (queue_match) { // Only print header once
                                std::cout << "\nComparing Huffman tree node queue orders (before tree building)..." << std::endl;
                            }
                            std::cerr << "Error: Leaf node data mismatch! Original: " << static_cast<int>(original_node->data) << ", Decoded: " << static_cast<int>(decoded_node->data) << std::endl;
                        }
                        queue_match = false;
                    }
                    if (original_node->frequency != decoded_node->frequency) {
                        if (ENABLE_QUEUE_ORDER_COMPARISON) {
                            if (queue_match) { // Only print header once
                                std::cout << "\nComparing Huffman tree node queue orders (before tree building)..." << std::endl;
                            }
                            std::cerr << "Error: Leaf node frequency mismatch for data " << static_cast<int>(original_node->data) << "! Original: " << original_node->frequency << ", Decoded: " << decoded_node->frequency << std::endl;
                        }
                        queue_match = false;
                    }
                }
                
                // Remove the top elements from both queues
                tree_queue.pop();
                decode_tree_queue.pop();
                
                // If we already found a mismatch, break early
                if (!queue_match) break;
            }
        }
        
        if (ENABLE_QUEUE_ORDER_COMPARISON) {
            if (queue_match) {
                std::cout << "Success: Node queues match before tree building!" << std::endl;
            } else {
                std::cerr << "Error: Node queues do not match before tree building!" << std::endl;
            }
        }
        
        // Validate tree construction
        bool original_tree_valid = validate_huffman_tree(tree, ENABLE_TREE_VALIDATION);
        bool decoded_tree_valid = validate_huffman_tree(decode_tree, ENABLE_TREE_VALIDATION);
        
        if (ENABLE_TREE_VALIDATION) {
            if (original_tree_valid) {
                std::cout << "Success: Original Huffman tree construction is valid!" << std::endl;
            } else {
                std::cerr << "Error: Original Huffman tree construction is invalid!" << std::endl;
            }
            
            if (decoded_tree_valid) {
                std::cout << "Success: Decoded Huffman tree construction is valid!" << std::endl;
            } else {
                std::cerr << "Error: Decoded Huffman tree construction is invalid!" << std::endl;
            }
        }
        
        // Compare code maps
        const auto& original_code_map = tree.get_code_map();
        const auto& decoded_code_map = decode_tree.get_code_map();
        
        bool code_map_match = true;
        
        if (original_code_map.size() != decoded_code_map.size()) {
            if (ENABLE_CODE_MAP_COMPARISON) {
                std::cout << "\nComparing code maps..." << std::endl;
                std::cerr << "Error: Code map size mismatch! Original: " << original_code_map.size() << ", Decoded: " << decoded_code_map.size() << std::endl;
            }
            code_map_match = false;
        } else {
            for (const auto& pair : original_code_map) {
                u8 value = pair.first;
                const auto& code_info = pair.second;
                u64 code = code_info.first;
                u8 code_length = code_info.second;
                
                if (decoded_code_map.find(value) == decoded_code_map.end()) {
                    if (ENABLE_CODE_MAP_COMPARISON) {
                        if (code_map_match) { // Only print header once
                            std::cout << "\nComparing code maps..." << std::endl;
                        }
                        std::cerr << "Error: Value " << static_cast<int>(value) << " not found in decoded code map!" << std::endl;
                    }
                    code_map_match = false;
                } else {
                    const auto& decoded_code_info = decoded_code_map.at(value);
                    u64 decoded_code = decoded_code_info.first;
                    u8 decoded_code_length = decoded_code_info.second;
                    
                    if (decoded_code_length != code_length) {
                        if (ENABLE_CODE_MAP_COMPARISON) {
                            if (code_map_match) { // Only print header once
                                std::cout << "\nComparing code maps..." << std::endl;
                            }
                            std::cerr << "Error: Code length mismatch for value " << static_cast<int>(value) << "! Original: " << static_cast<int>(code_length) << ", Decoded: " << static_cast<int>(decoded_code_length) << std::endl;
                        }
                        code_map_match = false;
                    }
                    if (decoded_code != code) {
                        if (ENABLE_CODE_MAP_COMPARISON) {
                            if (code_map_match) { // Only print header once
                                std::cout << "\nComparing code maps..." << std::endl;
                            }
                            std::cerr << "Error: Code mismatch for value " << static_cast<int>(value) << "! Original: 0x" << std::hex << code << ", Decoded: 0x" << decoded_code << std::dec << std::endl;
                        }
                        code_map_match = false;
                    }
                }
            }
        }
        
        if (ENABLE_CODE_MAP_COMPARISON) {
            if (code_map_match) {
                std::cout << "Success: Code maps match!" << std::endl;
            } else {
                std::cerr << "Error: Code maps do not match!" << std::endl;
            }
        }
        
        // Step 4: Verify by rebuilding tree from decoded frequency map
        if (ENABLE_RECHECK_CODE_MAP_COMPARISON) {
            std::cout << "\nVerifying by rebuilding tree from decoded frequency map..." << std::endl;
        }
        
        // Create a new Huffman tree using the decoded frequency map
        HuffmanTree<u8> recheck_tree;
        
        // Input data to Huffman tree using the frequency map overload
        recheck_tree.input_data(decoded_freq_map);
        
        // Build Huffman tree
        recheck_tree.spawnTree();
        
        // Get rechecked code map
        const auto& rechecked_code_map = recheck_tree.get_code_map();
        
        // Compare rechecked code map with original code map
        bool rechecked_code_map_match = true;
        
        if (original_code_map.size() != rechecked_code_map.size()) {
            if (ENABLE_RECHECK_CODE_MAP_COMPARISON) {
                std::cout << "\nComparing rechecked code map with original code map..." << std::endl;
                std::cerr << "Error: Rechecked code map size mismatch! Original: " << original_code_map.size() << ", Rechecked: " << rechecked_code_map.size() << std::endl;
            }
            rechecked_code_map_match = false;
        } else {
            for (const auto& pair : original_code_map) {
                u8 value = pair.first;
                const auto& code_info = pair.second;
                u64 code = code_info.first;
                u8 code_length = code_info.second;
                
                if (rechecked_code_map.find(value) == rechecked_code_map.end()) {
                    if (ENABLE_RECHECK_CODE_MAP_COMPARISON) {
                        if (rechecked_code_map_match) { // Only print header once
                            std::cout << "\nComparing rechecked code map with original code map..." << std::endl;
                        }
                        std::cerr << "Error: Value " << static_cast<int>(value) << " not found in rechecked code map!" << std::endl;
                    }
                    rechecked_code_map_match = false;
                } else {
                    const auto& rechecked_code_info = rechecked_code_map.at(value);
                    u64 rechecked_code = rechecked_code_info.first;
                    u8 rechecked_code_length = rechecked_code_info.second;
                    
                    if (rechecked_code_length != code_length) {
                        if (ENABLE_RECHECK_CODE_MAP_COMPARISON) {
                            if (rechecked_code_map_match) { // Only print header once
                                std::cout << "\nComparing rechecked code map with original code map..." << std::endl;
                            }
                            std::cerr << "Error: Rechecked code length mismatch for value " << static_cast<int>(value) << "! Original: " << static_cast<int>(code_length) << ", Rechecked: " << static_cast<int>(rechecked_code_length) << std::endl;
                        }
                        rechecked_code_map_match = false;
                    }
                    if (rechecked_code != code) {
                        if (ENABLE_RECHECK_CODE_MAP_COMPARISON) {
                            if (rechecked_code_map_match) { // Only print header once
                                std::cout << "\nComparing rechecked code map with original code map..." << std::endl;
                            }
                            std::cerr << "Error: Rechecked code mismatch for value " << static_cast<int>(value) << "! Original: 0x" << std::hex << code << ", Rechecked: 0x" << rechecked_code << std::dec << std::endl;
                        }
                        rechecked_code_map_match = false;
                    }
                }
            }
        }
        
        if (ENABLE_RECHECK_CODE_MAP_COMPARISON) {
            if (rechecked_code_map_match) {
                std::cout << "Success: Rechecked code map matches original code map!" << std::endl;
            } else {
                std::cerr << "Error: Rechecked code map does not match original code map!" << std::endl;
            }
        }
        
        // Resume original decompression process
        
        // Decode bitstream
        std::vector<u8> decoded_data = BitStream<u8>(decode_tree.get_root()).decode(encoded_bits, original_bmp->bit_num);
        
        std::cout << "Decompression completed." << std::endl;
        std::cout << "Decompressed size: " << decoded_data.size() << " bytes" << std::endl;
        
        // Step 4: Compare original and decompressed data
        if (ENABLE_FILE_COMPARISON) {
            std::cout << "\nComparing original and decompressed data..." << std::endl;
        }
        
        bool file_match = true;
        u64 mismatch_count = 0;
        const u64 max_mismatches = 10; // Only show first 10 mismatches
        
        if (original_bmp->bit_num != decoded_data.size()) {
            if (ENABLE_FILE_COMPARISON) {
                std::cerr << "Error: Size mismatch! Original: " << original_bmp->bit_num << " bytes, Decompressed: " << decoded_data.size() << " bytes" << std::endl;
            }
            file_match = false;
        } else {
            for (u32 i = 0; i < original_bmp->bit_num; i++) {
                if (original_bmp->filemap[i] != decoded_data[i]) {
                    mismatch_count++;
                    if (ENABLE_FILE_COMPARISON && mismatch_count <= max_mismatches) {
                        std::cerr << "Mismatch at position " << i << ": Original 0x" << std::hex << static_cast<int>(original_bmp->filemap[i]) << ", Decompressed 0x" << static_cast<int>(decoded_data[i]) << std::dec << std::endl;
                    }
                }
            }
            
            file_match = (mismatch_count == 0);
        }
        
        if (ENABLE_FILE_COMPARISON) {
            if (file_match) {
                std::cout << "SUCCESS: Original and decompressed data are identical!" << std::endl;
            } else {
                std::cerr << "ERROR: Found " << mismatch_count << " mismatches between original and decompressed data!" << std::endl;
                if (mismatch_count > max_mismatches) {
                    std::cerr << "(Only first " << max_mismatches << " mismatches shown)" << std::endl;
                }
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Exception during compression/decompression: " << e.what() << std::endl;
    }
    
    // Clean up
    std::cout << "Cleaning up resources..." << std::endl;
    
    // 手动释放original_bmp中的filemap向量
    if (original_bmp != nullptr) {
        original_bmp->filemap.clear();
        original_bmp->filemap.shrink_to_fit();
        delete original_bmp;
        original_bmp = nullptr;
    }
    
    std::cout << "All resources cleaned up successfully!" << std::endl;
    std::cout << "Program completed successfully!" << std::endl;
    
    // 强制刷新所有输出
    std::cout.flush();
    
    return 0;
}
