#include "FileHeadWriter.h"

FileHeadWriter::FileHeadWriter(const std::string& filename) 
    : FileWriter(filename), m_filename(filename) {
    m_file_format = get_file_format(filename);
}

FileHeadWriter::~FileHeadWriter() {
    // 析构函数
}

bool FileHeadWriter::writeHead(const std::vector<u8>& head) {
    // 写入头部数据
    for (const auto& byte : head) {
        writeu8(byte);
    }
    return true;
}

bool FileHeadWriter::writeFormattedHeader(const std::unordered_map<std::string, u64>& headerValues) {
    // 根据文件格式写入特定格式的头部
    for (const auto& format : file_format_list) {
        if (format.suffix_name == m_filename.substr(m_filename.find_last_of('.'))) {
            for (size_t i = 0; i < format.field_count; ++i) {
                const auto& field = format.fields[i];
                
                if (field.size > 0) {
                    // 在传入的headerValues中查找对应的值
                    auto it = headerValues.find(std::string(field.name));
                    u64 value = (it != headerValues.end()) ? it->second : 0;
                    
                    // 根据字段大小写入相应字节数
                    switch (field.size) {
                        case 1:
                            writeu8(static_cast<u8>(value));
                            break;
                        case 2:
                            writeu16(static_cast<u16>(value));
                            break;
                        case 4:
                            writeu32(static_cast<u32>(value));
                            break;
                        case 8:
                            writeu64(value);
                            break;
                        default:
                            // 对于其他大小，逐字节写入
                            for (int j = 0; j < field.size; ++j) {
                                writeu8(static_cast<u8>((value >> (j * 8)) & 0xFF));
                            }
                            break;
                    }
                } else if (field.size == -1) {
                    // -1表示变长字段，在此不处理，留给具体业务逻辑处理
                }
            }
            break;
        }
    }
    
    return true;
}

bool FileHeadWriter::writeBlock(const std::vector<u8>& data) {
    // 直接写入整个数据块
    for (const auto& byte : data) {
        writeu8(byte);
    }
    return true;
}

