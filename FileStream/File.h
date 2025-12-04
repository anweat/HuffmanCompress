#ifndef FILE_H
#define FILE_H

#include <fstream>
#include <string>
#include <unordered_map>
#include "sysdetect.h"
#include "FileFormat.h"


enum FileMode
{
    READ,
    WRITE,
    READ_WRITE
};

class File
{
    typedef unsigned long u32;

private:
    int systemBits;
    bool isLittleEndian;
    bool isBigEndian;

protected:
    std::string filename;
    
    // 将文件流设为protected以便子类访问
    std::ifstream file_in_stream;
    std::ofstream file_out_stream;
    std::unordered_map<std::string, int> header;
    FileMode mode;

public:
    // 提供文件流的访问方法，确保安全访问
    const std::ifstream& getInputStream() const { return file_in_stream; }
    const std::ofstream& getOutputStream() const { return file_out_stream; }

    std::ifstream &getInputStream() { return file_in_stream; }
    std::ofstream& getOutputStream() { return file_out_stream; }

public:
    int getSystemBits() const { return systemBits; }
    bool isSystemLittleEndian() const { return isLittleEndian; }
    bool isSystemBigEndian() const { return isBigEndian; }

    const std::string &getFileName() const { return filename; }

    // 获取文件头信息
    const std::unordered_map<std::string, int> &getHeader() const {
        return header;
    }

    File(std::string filename) : filename(filename)
    {
        systemBits = GET_SYSTEM_BITS();
        isLittleEndian = IS_LITTLE_ENDIAN;
        isBigEndian = IS_BIG_ENDIAN;
        header = get_file_format(filename);
    }

    File(std::string filename, FileMode mode):File(filename){
        if(mode == FileMode::READ){
            file_in_stream.open(filename, std::ios::in | std::ios::binary);
            if(file_in_stream.is_open()){
                this->mode = FileMode::READ;
            }
            else{
                throw std::runtime_error("File open failed");
            }
        }
        else if(mode == FileMode::WRITE){
            file_out_stream.open(filename, std::ios::out | std::ios::binary);
            if(file_out_stream.is_open()){
                this->mode = FileMode::WRITE;
            }
            else{
                throw std::runtime_error("File open failed");
            }
        }
        else if(mode == FileMode::READ_WRITE){
            // 同时打开输入流和输出流
            file_in_stream.open(filename, std::ios::in | std::ios::binary);
            file_out_stream.open(filename, std::ios::out | std::ios::binary);
            if(file_in_stream.is_open() && file_out_stream.is_open()){
                this->mode = FileMode::READ_WRITE;
            }
            else{
                // 关闭已打开的流
                if(file_in_stream.is_open()) file_in_stream.close();
                if(file_out_stream.is_open()) file_out_stream.close();
                throw std::runtime_error("File open failed in READ_WRITE mode");
            }
        }
    }

    // 析构函数，关闭文件
    virtual ~File() {
        if (file_in_stream.is_open()) {
            file_in_stream.close();
        }
        if (file_out_stream.is_open()) {
            file_out_stream.close();
        }
    }

    // 检查文件是否打开
    bool isOpen() const {
        // 根据模式检查相应的文件流是否打开
        if (mode == FileMode::READ) {
            return file_in_stream.is_open();
        } else if (mode == FileMode::WRITE) {
            return file_out_stream.is_open();
        } else if (mode == FileMode::READ_WRITE) {
            return file_in_stream.is_open() && file_out_stream.is_open();
        }
        return false;
    }

    bool isClosed() const {
        return !isOpen();
    }

    // 获取当前模式
    FileMode getMode() const {
        return mode;
    }

    // 获取文件大小
    size_t getFileSize()
    {
        if (mode == FileMode::READ || mode == FileMode::READ_WRITE)
        {
            std::streampos current = file_in_stream.tellg();
            file_in_stream.seekg(0, std::ios::end);
            std::streampos size = file_in_stream.tellg();
            file_in_stream.seekg(current);
            return static_cast<size_t>(size);
        }
        return 0;
    }
};

namespace Swap{
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;

    // 单字节不需要交换
    inline u8 byteSwap(u8 value) {
        return value;
    }

    // 双字节交换
    inline u16 byteSwap(u16 value) {
        return (value << 8) | (value >> 8);
    }

    // 四字节交换
    inline u32 byteSwap(u32 value) {
        return ((value << 24) | 
                ((value << 8) & 0x00FF0000) | 
                ((value >> 8) & 0x0000FF00) | 
                (value >> 24));
    }

    // 八字节交换
    inline u64 byteSwap(u64 value) {
        return ((value << 56) | 
                ((value << 40) & 0x00FF000000000000) | 
                ((value << 24) & 0x0000FF0000000000) | 
                ((value << 8) & 0x000000FF00000000) | 
                ((value >> 8) & 0x00000000FF000000) | 
                ((value >> 24) & 0x0000000000FF0000) | 
                ((value >> 40) & 0x000000000000FF00) | 
                (value >> 56));
    }
}

#endif
