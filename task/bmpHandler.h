#ifndef BMPHANDLER_H
#define BMPHANDLER_H

#include "../FileStream/FileHeadReader.h"
#include "../FileStream/FileHeadWriter.h"
#include "../FileStream/FileFormat.h"
#include "../logger/Logger.h"
#include <unordered_map>


// 使用与FileFormat.h中一致的bit定义
typedef unsigned char bit;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

class bmpBase{
public:
    u32 bit_num;

    std::vector<u8> filemap;

    virtual void readBitmapData(FileHeadReader &reader) = 0;

    virtual void writeData(FileWriter &writer) const = 0;
};

class bmp : public bmpBase{
public:

    void readBitmapData(FileHeadReader &reader) override{
      Logger::getInstance().info("开始读取BMP位图数据");
      bit_num = reader.getFileSize();
      Logger::getInstance().debug("BMP文件大小: " + std::to_string(bit_num));
      filemap.resize(bit_num);
      
      // 重置文件指针到开始位置
      reader.getFile().getInputStream().seekg(0, std::ios::beg);
      
      for(u32 i = 0; i < bit_num; i++){
        filemap[i] = reader.readu8();
      }
      Logger::getInstance().info("成功读取BMP位图数据");
    }

    void writeData(FileWriter &writer) const override{
      Logger::getInstance().info("开始写入BMP数据");
      for(u32 i = 0; i < bit_num; i++){
        writer.writeu8(filemap[i]);
      }
      Logger::getInstance().info("成功写入BMP数据");
    }
};

class bmpHandler
{

public:
  static bool huf2bmp_start(const std::string &filename, const std::string &output_filename, double *process); // 图像处理任务（加载huf文件，构造huf文件体，读取数据-频数对构造huffman树，还原位流，保存bmp文件）

  static bmp *load(const std::string &filename) {
    Logger::getInstance().info("正在加载BMP文件: " + filename);
    FileHeadReader reader(filename);
    bmp* bmpFile = new bmp();
    bmpFile->readBitmapData(reader);
    Logger::getInstance().info("成功加载BMP文件: " + filename);

    return bmpFile;
  }

  // 保存bmp文件（读取bmpFile，保存到filename中
  static bool save(const std::string &filename, const bmp *bmpFile){
    Logger::getInstance().info("正在保存BMP文件: " + filename);
    FileWriter writer(filename);
    bmpFile->writeData(writer);
    bool result = writer.close();
    if (result) {
      Logger::getInstance().info("成功保存BMP文件: " + filename);
    } else {
      Logger::getInstance().error("保存BMP文件失败: " + filename);
    }
    return result;
  }
};

#endif