#include "FileReader.h"
#include <stdexcept>

FileReader::FileReader(std::string filename) : file(filename, FileMode::READ) {
    // 构造函数，使用File类的READ模式打开文件
}

FileReader::~FileReader() {
    // 析构函数，基类File的析构函数会处理文件关闭
}

// 读取单字节
u8 FileReader::readu8() {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for reading");
    }
    
    u8 value;
    file.getInputStream().read(reinterpret_cast<char*>(&value), sizeof(u8));
    
    if (file.getInputStream().fail()) {
        throw std::runtime_error("Failed to read u8 value");
    }
    
    return value;
}

// 读取双字节，处理字节序
u16 FileReader::readu16() {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for reading");
    }
    
    u16 value;
    file.getInputStream().read(reinterpret_cast<char*>(&value), sizeof(u16));
    
    if (file.getInputStream().fail()) {
        throw std::runtime_error("Failed to read u16 value");
    }
    
    // 假设文件数据是小端序存储
    // 如果系统是大端序，需要交换字节
    if (file.isSystemBigEndian()) {
        value = byteSwap(value);
    }
    
    return value;
}

// 读取四字节，处理字节序
u32 FileReader::readu32() {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for reading");
    }
    
    u32 value;
    file.getInputStream().read(reinterpret_cast<char*>(&value), sizeof(u32));
    
    if (file.getInputStream().fail()) {
        throw std::runtime_error("Failed to read u32 value");
    }
    
    // 假设文件数据是小端序存储
    // 如果系统是大端序，需要交换字节
    if (file.isSystemBigEndian()) {
        value = byteSwap(value);
    }
    
    return value;
}

// 读取八字节，处理字节序
u64 FileReader::readu64() {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for reading");
    }
    
    u64 value;
    file.getInputStream().read(reinterpret_cast<char*>(&value), sizeof(u64));
    
    if (file.getInputStream().fail()) {
        throw std::runtime_error("Failed to read u64 value");
    }
    
    // 假设文件数据是小端序存储
    // 如果系统是大端序，需要交换字节
    if (file.isSystemBigEndian()) {
        value = byteSwap(value);
    }
    
    return value;
}

size_t FileReader::tell(){
    return static_cast<size_t>(file.getInputStream().tellg());
}