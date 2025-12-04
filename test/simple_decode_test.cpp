#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include "huffman/huffmantree.h"
#include "huffman/bitstream.h"

using namespace std;

typedef unsigned char u8;
typedef unsigned long long u64;

// 简化的HUF文件加载函数
struct SimpleHufData {
    vector<u8> bitset;
    u64 bitset_size;
    u64 bit_num;
    u64 key_num;
    u8 key_size;
    u8 value_size;
    unordered_map<u8, u64> frequency_map;
};

SimpleHufData load_simple_huf(const string& filename) {
    SimpleHufData data;
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Failed to open file");
    }

    // 读取HUF文件头
    u16 huf_type;
    u32 file_size;
    u32 reserved;
    u8 key_size;
    u8 value_size;
    u32 key_num;
    u64 bit_num;
    u64 bitset_size;

    file.read(reinterpret_cast<char*>(&huf_type), sizeof(huf_type));
    file.read(reinterpret_cast<char*>(&file_size), sizeof(file_size));
    file.read(reinterpret_cast<char*>(&reserved), sizeof(reserved));
    file.read(reinterpret_cast<char*>(&key_size), sizeof(key_size));
    file.read(reinterpret_cast<char*>(&value_size), sizeof(value_size));
    file.read(reinterpret_cast<char*>(&key_num), sizeof(key_num));
    file.read(reinterpret_cast<char*>(&bit_num), sizeof(bit_num));
    file.read(reinterpret_cast<char*>(&bitset_size), sizeof(bitset_size));

    data.key_size = key_size;
    data.value_size = value_size;
    data.key_num = key_num;
    data.bit_num = bit_num;
    data.bitset_size = bitset_size;

    // 读取键值对
    for (u64 i = 0; i < key_num; ++i) {
        u8 key;
        u64 value;
        
        file.read(reinterpret_cast<char*>(&key), key_size);
        
        if (value_size == 1) {
            u8 v;
            file.read(reinterpret_cast<char*>(&v), sizeof(v));
            value = v;
        } else if (value_size == 2) {
            u16 v;
            file.read(reinterpret_cast<char*>(&v), sizeof(v));
            value = v;
        } else if (value_size == 4) {
            u32 v;
            file.read(reinterpret_cast<char*>(&v), sizeof(v));
            value = v;
        } else if (value_size == 8) {
            u64 v;
            file.read(reinterpret_cast<char*>(&v), sizeof(v));
            value = v;
        }
        
        data.frequency_map[key] = value;
    }

    // 读取位流数据
    data.bitset.resize(bitset_size);
    file.read(reinterpret_cast<char*>(data.bitset.data()), bitset_size);

    file.close();
    return data;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <input.huf> <output.bmp>" << endl;
        return 1;
    }

    string input_file = argv[1];
    string output_file = argv[2];

    try {
        // 加载HUF文件
        SimpleHufData huf_data = load_simple_huf(input_file);
        
        cout << "HUF file loaded successfully!" << endl;
        cout << "KeyNum: " << huf_data.key_num << endl;
        cout << "BitNum: " << huf_data.bit_num << endl;
        cout << "BitsetSize: " << huf_data.bitset_size << endl;
        cout << "Frequency map size: " << huf_data.frequency_map.size() << endl;

        // 创建Huffman树
        HuffmanTree<u8> tree;
        tree.input_data(huf_data.frequency_map);
        tree.spawnTree();
        
        cout << "Huffman tree created successfully!" << endl;

        // 解码位流
        BitStream<u8> decode_stream(tree.get_root());
        cout << "Starting decode..." << endl;
        vector<u8> decode_data = decode_stream.decode(huf_data.bitset, huf_data.bit_num);
        
        cout << "Decode completed!" << endl;
        cout << "Expected decoded size: " << huf_data.bit_num << endl;
        cout << "Actual decoded size: " << decode_data.size() << endl;

        if (decode_data.size() != huf_data.bit_num) {
            cout << "ERROR: Decoded size mismatch!" << endl;
        } else {
            cout << "SUCCESS: Decoded size matches expected!" << endl;
        }

        // 保存解码后的数据
        cout << "Saving to file..." << endl;
        ofstream outfile(output_file, ios::binary);
        if (!outfile) {
            throw runtime_error("Failed to open output file");
        }

        outfile.write(reinterpret_cast<const char*>(decode_data.data()), decode_data.size());
        outfile.close();

        // 检查文件大小
        ifstream checkfile(output_file, ios::binary | ios::ate);
        streamsize file_size = checkfile.tellg();
        checkfile.close();
        
        cout << "Output file size: " << file_size << " bytes" << endl;
        
        if (file_size == decode_data.size()) {
            cout << "SUCCESS: File saved correctly!" << endl;
        } else {
            cout << "ERROR: File size mismatch!" << endl;
        }

    } catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}