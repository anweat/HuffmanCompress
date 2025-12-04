#ifndef FILEREADER_H
#define FILEREADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "File.h"

using namespace Swap;

class FileReader{

protected:
    File file;

     

public:
    // 读取单字节
    u8 readu8();

    // 读取双字节，处理字节序
    u16 readu16();

    // 读取四字节，处理字节序
    u32 readu32();

    // 读取八字节，处理字节序
    u64 readu64();
    FileReader(std::string filename);

    virtual ~FileReader();

    size_t tell();

    File &getFile()
    {
        return file;
    };
};

#endif