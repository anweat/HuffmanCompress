#ifndef FILEHEADREADER_H
#define FILEHEADREADER_H

#include "FileReader.h"
#include "FileFormat.h"
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <vector>
#include <functional>

// 定义头部映射类型
typedef std::unordered_map<std::string, unsigned int> HeaderMap;
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned long long u64;

class FileHeadReader : public FileReader{
protected:

    std::string filename;

public:
    // 构造函数
    explicit FileHeadReader(const std::string& filename);

    // 虚析构函数
    virtual ~FileHeadReader();

    // 移动到数据部分
    bool toDataHeader();

    int getHeadSize();

    std::unordered_map<std::string, u64>  getHeader();

    // 获取文件大小
    size_t getFileSize();

    // 重置文件读取指针到文件开头
    void reset();

    // 获取当前文件位置  
    size_t tell();

private:

    std::unordered_map<std::string, u64> readCompleteHeader();

    std::unordered_map<std::string, u64> header;

    std::unordered_map<std::string, int> m_header_format;


    int m_head_size;

    bool m_is_header_read;

};

#endif // FILEHEADREADER_H
