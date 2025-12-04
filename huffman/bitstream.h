#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <unordered_map>
#include "huffmantree.h"
#include "../logger/Logger.h"

template <typename T>
class BitStream
{
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;

public:
    // 编码模式构造函数
    BitStream(std::unordered_map<T, std::pair<u64, u8>> i_code_map)
        : code_map(std::move(i_code_map)), root(nullptr)
    {}

    // 解码模式构造函数
    BitStream(node<T> *i_root)
        : root(i_root)
    {}

    // 编码模式，也可以做解码模式

    std::vector<T> decode(std::vector<u8> bytes, u64 code_num)
    {
        std::vector<T> result;
        result.reserve(code_num);

        if (root == nullptr)
        {
            throw std::runtime_error("Huffman tree root is null");
        }

        node<T> *current = root;
        u64 decoded_count = 0;

        // 遍历每个字节
        for (const u8 &byte : bytes)
        {
            // 从最高位到最低位处理每个bit
            for (int i = 7; i >= 0 && decoded_count < code_num; i--)
            {
                // 提取当前bit (0或1)
                u8 bit = (byte >> i) & 1;

                // 根据bit值在树中移动
                if (bit == 0)
                {
                    current = current->left_child;
                }
                else
                {
                    current = current->right_child;
                }

                // 检查是否到达叶节点
                if (current != nullptr && current->is_leaf)
                {
                    result.push_back(current->data);
                    decoded_count++;
                    current = root; // 回到根节点继续解码
                }else if (current == nullptr){
                    Logger::getInstance().error("Invalid bit sequence");
                    throw std::runtime_error("Invalid bit sequence");
                }
            }

            // 如果已经解码完所有数据，提前结束
            if (decoded_count >= code_num)
            {
                break;
            }
        }
        if (decoded_count != code_num)
        {
            Logger::getInstance().error("Decoded count not equal to code number");
            throw std::runtime_error("Decoded count not equal to code number");
        }

        return result;
    }

    std::vector<u8> encode(std::vector<T> data)
    {
        std::vector<u8> result;

        u8 buffer = 0;      // 位缓冲区
        u8 buffer_bits = 0; // 缓冲区中已使用的位数

        // 处理每个数据元素
        for (const T &item : data)
        {
            // 查找该数据的编码
            auto it = code_map.find(item);
            if (it == code_map.end())
            {
                throw std::runtime_error("Code not found for data element");
            }

            u64 code = it->second.first;        // Huffman编码
            u8 code_length = it->second.second; // 编码长度

            // 从最高位到最低位处理编码的每一位
            for (int i = code_length - 1; i >= 0; i--)
            {
                // 提取当前位
                u8 bit = (code >> i) & 1;

                // 将位添加到缓冲区
                buffer = (buffer << 1) | bit;
                buffer_bits++;

                // 如果缓冲区满了8位，就写入结果
                if (buffer_bits == 8)
                {
                    result.push_back(buffer);
                    buffer = 0;
                    buffer_bits = 0;
                }
            }
        }
        // 处理最后不完整的字节
        if (buffer_bits > 0)
        {
            buffer <<= (8 - buffer_bits); // 左移以对齐高位
            result.push_back(buffer);
        }

        return result;
    }

private:
    std::unordered_map<T, std::pair<u64, u8>> code_map; // 编码，编码长度

    node<T> *root;
};

template class BitStream<unsigned char>;
template class BitStream<unsigned short>;
template class BitStream<unsigned int>;
#endif