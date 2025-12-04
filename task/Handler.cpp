#include "bmpHandler.h"
#include "hufHandler.h"
#include "huffmantree.h"
#include "bitstream.h"

bool bmpHandler::huf2bmp_start(const std::string &filename, const std::string &output_filename, double *process){
    Logger::getInstance().info("开始HUF到BMP转换任务: " + filename + " -> " + output_filename);
    huf *hufFile = hufHandler::load(filename);
    
    Logger::getInstance().debug("创建霍夫曼树");
    HuffmanTree<u8> tree = HuffmanTree<u8>();

    std::unordered_map<u8, u64> key_value;

    Logger::getInstance().debug("转换键值对数据格式");
    for(auto i : hufFile->key_value_data){
        key_value.insert(std::make_pair(static_cast<u8>(i.first), i.second));
    }

    tree.input_data(key_value);
    
    Logger::getInstance().debug("构建霍夫曼树");
    tree.spawnTree();

    Logger::getInstance().debug("创建解码流");
    BitStream<u8> decode_stream(tree.get_root());

    Logger::getInstance().debug("解码位流数据");
    std::vector<u8> decode_data = decode_stream.decode(hufFile->bitset, hufFile->bit_num);

    Logger::getInstance().debug("创建BMP文件对象");
    bmp* bmpFile = new bmp();

    bmpFile->bit_num = decode_data.size();
    Logger::getInstance().debug("BMP文件位数量: " + std::to_string(bmpFile->bit_num));

    bmpFile->filemap = decode_data;

    Logger::getInstance().debug("保存BMP文件");
    bool result = bmpHandler::save(output_filename, bmpFile);

    delete bmpFile;
    delete hufFile;
    
    Logger::getInstance().info("完成HUF到BMP转换任务");
    return result;
}

bool hufHandler::bmp2huf_start(const std::string &filename, const std::string &output_filename, double *process)
{
    Logger::getInstance().info("开始BMP到HUF转换任务: " + filename + " -> " + output_filename);
    Logger::getInstance().debug("加载BMP文件");
    bmp *bmpFile = bmpHandler::load(filename);
    
    Logger::getInstance().debug("创建霍夫曼树");
    HuffmanTree<u8> tree = HuffmanTree<u8>();
    
    Logger::getInstance().debug("输入数据到霍夫曼树");
    for (u32 num = 0; num < bmpFile->bit_num; num++){
        tree.input_data(bmpFile->filemap[num]);
    }
    
    Logger::getInstance().debug("构建霍夫曼树");
    tree.spawnTree();

    Logger::getInstance().debug("编码位流数据");
    std::vector<u8> bitset = BitStream<u8>(tree.get_code_map()).encode(bmpFile->filemap);

    Logger::getInstance().debug("创建HUF文件对象");
    huf* hufFile = new huf();
    hufFile->bit_num = bmpFile->bit_num;
    hufFile->bitset  = bitset;
    hufFile->bitset_size = bitset.size();
    hufFile->key_num = tree.get_code_map().size();
    hufFile->key_size = sizeof(unsigned char); // 对于u8类型，key_size总是1
    hufFile->value_size = tree.get_frequency_length();
    
    Logger::getInstance().debug("转换键值对数据格式");
    std::unordered_map<u64,u64> key_value_data;
    for(auto i :tree.get_frequency_map()){
        key_value_data.insert(std::make_pair(static_cast<u64>(i.first),i.second));
    }
    hufFile->key_value_data = key_value_data;

    Logger::getInstance().debug("保存HUF文件");
    hufHandler::save(output_filename, hufFile);
    
    delete bmpFile;
    delete hufFile;
    
    Logger::getInstance().info("完成BMP到HUF转换任务");
    return true;
}