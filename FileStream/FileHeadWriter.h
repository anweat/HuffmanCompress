#ifndef FILEHEADWRITER_H
#define FILEHEADWRITER_H

#include "FileWriter.h"
#include "FileFormat.h"
#include <string>
#include <unordered_map>
#include <vector>

class FileHeadWriter : public FileWriter{
private:
    std::string m_filename; // 存储文件名

    std::unordered_map<std::string, int> m_file_format;

public:
    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;

    // 写入头部数据
    bool writeHead(const std::vector<u8>& head);

    // 写入特定格式的头部
    bool writeFormattedHeader(const std::unordered_map<std::string,u64>& headerValues);
    
    //直接写入整个数据
    bool writeBlock(const std::vector<u8>& data);

    // 构造函数
    FileHeadWriter(const std::string& filename);

    // 析构函数
    virtual ~FileHeadWriter();

    
};

#endif
