#include "FileWriter.h"
#include <stdexcept>

// 写入单字节
void FileWriter::writeu8(u8 value) {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for writing");
    }
    
    file.getOutputStream().write(reinterpret_cast<const char*>(&value), sizeof(u8));
    
    if (file.getOutputStream().fail()) {
        throw std::runtime_error("Failed to write u8 value");
    }
}

// 写入双字节，处理字节序
void FileWriter::writeu16(u16 value) {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for writing");
    }
    
    // 假设文件数据需要小端序存储
    // 如果系统是大端序，需要交换字节
    if (file.isSystemBigEndian()) {
        value = byteSwap(value);
    }
    
    file.getOutputStream().write(reinterpret_cast<const char*>(&value), sizeof(u16));
    
    if (file.getOutputStream().fail()) {
        throw std::runtime_error("Failed to write u16 value");
    }
}

// 写入四字节，处理字节序
void FileWriter::writeu32(u32 value) {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for writing");
    }
    
    // 假设文件数据需要小端序存储
    // 如果系统是大端序，需要交换字节
    if (file.isSystemBigEndian()) {
        value = byteSwap(value);
    }
    
    file.getOutputStream().write(reinterpret_cast<const char*>(&value), sizeof(u32));
    
    if (file.getOutputStream().fail()) {
        throw std::runtime_error("Failed to write u32 value");
    }
}

// 写入八字节，处理字节序
void FileWriter::writeu64(u64 value) {
    if (!file.isOpen()) {
        throw std::runtime_error("File not open for writing");
    }
    
    // 假设文件数据需要小端序存储
    // 如果系统是大端序，需要交换字节
    if (file.isSystemBigEndian()) {
        value = byteSwap(value);
    }
    
    file.getOutputStream().write(reinterpret_cast<const char*>(&value), sizeof(u64));
    
    if (file.getOutputStream().fail()) {
        throw std::runtime_error("Failed to write u64 value");
    }
}

bool FileWriter::close(){
    if (file.isOpen()) {
        // 由于File类的析构函数会自动关闭文件，我们只需要返回true
        return true;
    }
    return false;
}