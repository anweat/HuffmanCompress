#ifndef HUFMAN_HANDLER_H
#define HUFMAN_HANDLER_H

#include "../FileStream/FileHeadReader.h"
#include "../FileStream/FileHeadWriter.h"
#include "../FileStream/FileFormat.h"
#include "../logger/Logger.h"
#include <vector>
#include <unordered_map>


typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// 基类定义
class hufBase {
public:
    std::vector<u8> bitset;
    u64 bitset_size;     // 位集大小
    u64 bit_num;         // 位集对应像素数
    u64 key_num;         // 键值对数量
    u8 key_size;         // 键大小
    u8 value_size;       // 值大小
    
    virtual ~hufBase() = default;
    
    // 纯虚函数，用于读取数据
    virtual void readKeyValueData(FileHeadReader& reader) = 0;

    virtual void readBitsetData(FileHeadReader &reader) = 0;


    virtual void writeKeyValueData(FileWriter &writer) const = 0;

    virtual void writeBitsetData(FileWriter &writer) const = 0;
};

// 使用vector存储键值对的huf类
class huf : public hufBase {
public:
    // 使用vector存储键值对，而不是unordered_map
    // 每个元素是一对连续的键和值的字节数据
    std::unordered_map<u64,u64> key_value_data;
    
    // 实现读取键值对数据的函数
    void readKeyValueData(FileHeadReader& reader) override {
        Logger::getInstance().info("开始读取HUF键值对数据");
        for (u64 i = 0; i < key_num; ++i) {
            u64 key;
            switch(key_size){
                case 1:{
                    key = reader.readu8();
                    break;
                }
                case 2:{
                    key = reader.readu16();
                    break;
                }
                case 4:{
                    key = reader.readu32();
                    break;
                }
                case 8:{
                    key = reader.readu64();
                    break;
                }
            }
            u64 value;
            switch(value_size){
                case 1:{
                    value = reader.readu8();
                    break;
                }
                case 2:{
                    value = reader.readu16();
                    break;
                }
                case 4:{
                    value = reader.readu32();
                    break;
                }
                case 8:{
                    value = reader.readu64();
                    break;
                }
            }
            key_value_data.insert(std::make_pair(key, value));
        }
        Logger::getInstance().debug("读取了 " + std::to_string(key_num) + " 个键值对");
        Logger::getInstance().info("成功读取HUF键值对数据");
    }

    void writeKeyValueData(FileWriter &writer) const override {
        Logger::getInstance().info("开始写入HUF键值对数据");
        for(auto &pair : key_value_data){
           switch(key_size){
                case 1:{
                    u8 key = static_cast<u8>(pair.first);
                    writer.writeu8(key);
                    break;
                }
                case 2:{
                    u16 key = static_cast<u16>(pair.first);
                    writer.writeu16(key);
                    break;
                }
                case 4:{
                    u32 key = static_cast<u32>(pair.first);
                    writer.writeu32(key);
                    break;
                }
                case 8:{
                    writer.writeu64(pair.first);
                    break;
                }
           }

           switch(value_size){
                case 1:{
                    u8 value = static_cast<u8>(pair.second);
                    writer.writeu8(value);
                    break;
                }
                case 2:{
                    u16 value = static_cast<u16>(pair.second);
                    writer.writeu16(value);
                    break;
                }
                case 4:{
                    u32 value = static_cast<u32>(pair.second);
                    writer.writeu32(value);
                    break;
                }
                case 8:{
                    writer.writeu64(pair.second);
                    break;
                }
           }

        }
        Logger::getInstance().info("成功写入HUF键值对数据");
    }

    void readBitsetData(FileHeadReader &reader) override {
        Logger::getInstance().info("开始读取HUF位集数据");
        for(int i = 0; i < bitset_size; i++){
            bitset.push_back(reader.readu8());
        }
        Logger::getInstance().debug("读取了 " + std::to_string(bitset_size) + " 字节位集数据");
        Logger::getInstance().info("成功读取HUF位集数据");
    }

    void writeBitsetData(FileWriter &writer) const override {
        Logger::getInstance().info("开始写入HUF位集数据");
        for(int i = 0; i < bitset_size; i++){
            writer.writeu8(bitset[i]);
        }
        Logger::getInstance().info("成功写入HUF位集数据");
    }


};

class hufHandler
{
public:
    // 工厂函数，完全根据文件数据创建适当的huf对象
    static huf* load(const std::string &filename) {
        Logger::getInstance().info("正在加载HUF文件: " + filename);
        // 先读取文件头以获取key_size和value_size
        FileHeadReader reader(filename);
        std::unordered_map<std::string, u64> header = reader.getHeader();
        u8 key_size = header["keySize"];
        u8 value_size = header["valueSize"];
        u64 key_num = header["keyNum"];
        u64 bit_num = header["bitNum"];
        u64 bitset_size = header["bitsetSize"];
        
        Logger::getInstance().debug("HUF文件头信息 - KeySize: " + std::to_string(key_size) + 
                                   ", ValueSize: " + std::to_string(value_size) +
                                   ", KeyNum: " + std::to_string(key_num) +
                                   ", BitNum: " + std::to_string(bit_num) +
                                   ", BitSetSize: " + std::to_string(bitset_size));
        
        // 根据key_size和value_size创建适当的对象
        huf* hufFile = new huf();
        
        // 设置基本属性
        hufFile->key_size = key_size;
        hufFile->value_size = value_size;
        hufFile->key_num = key_num;
        hufFile->bit_num = bit_num;
        hufFile->bitset_size = bitset_size;
        
        // 移动到键值对数据部分
        reader.toDataHeader();
        
        // 读取键值对数据
        hufFile->readKeyValueData(reader);

        hufFile->readBitsetData(reader);
        
        Logger::getInstance().info("成功加载HUF文件: " + filename);
        return hufFile;
    }
    
public:
    static bool bmp2huf_start(const std::string &filename, const std::string &output_filename, double *process); // bmp加载器加载，读取头和像素数，遍历数据建树，生成位流，保存至huf文件

    // 保存HUF文件
    static bool save(const std::string &filename, const huf* hufFile) {
        Logger::getInstance().info("正在保存HUF文件: " + filename);
        u32 size = 32; // hufType(2B) + fileSize(4B) + reserved(4B) + keySize(1B) + valueSize(1B) + keyNum(4B) + bitNum(8B) + bitsetSize(8B) = 32B
        size += hufFile->bitset_size;
        size += hufFile->key_num * hufFile->key_size;
        size += hufFile->key_num * hufFile->value_size;
        FileWriter writer(filename);
        writer.writeu16(0x5546); // 'U'是0x55，'F'是0x46
        writer.writeu32(size);
        writer.writeu32(0);
        writer.writeu8(hufFile->key_size);
        writer.writeu8(hufFile->value_size);
        writer.writeu32(static_cast<u32>(hufFile->key_num));
        writer.writeu64(hufFile->bit_num);
        writer.writeu64(hufFile->bitset_size);
        hufFile->writeKeyValueData(writer);
        hufFile->writeBitsetData(writer);
        bool result = writer.close();
        if (result) {
          Logger::getInstance().info("成功保存HUF文件: " + filename);
        } else {
          Logger::getInstance().error("保存HUF文件失败: " + filename);
        }
        return result;
    }
};

#endif
