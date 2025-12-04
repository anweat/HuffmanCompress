#ifndef FILEWRITER_H
#define FILEWRITER_H

#include "File.h"

using namespace Swap;

class FileWriter{

    typedef unsigned char u8;
    typedef unsigned short u16;
    typedef unsigned int u32;
    typedef unsigned long long u64;

public:
    FileWriter(std::string filename) : file(filename, FileMode::WRITE) {};

    void writeu8(u8 value);
    void writeu16(u16 value);
    void writeu32(u32 value);
    void writeu64(u64 value);

    bool close();

    File& getFile() {
            return file;
        };
protected : File file;


};

#endif
