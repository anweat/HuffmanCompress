#include "FileHeadReader.h"

FileHeadReader::FileHeadReader(const std::string& filename) 
    : FileReader(filename), filename(filename) {
    m_header_format = get_file_format(filename);
    
    // 计算头部大小
    m_head_size = 0;
    for (const auto& field : m_header_format) {
        if (field.second > 0) {  // 忽略大小为-1的字段（如数据字段）
            m_head_size += field.second;
        }
    }
    
    // 读取完整的头部
    header = readCompleteHeader();
}

FileHeadReader::~FileHeadReader() {
    // 析构函数
}

bool FileHeadReader::toDataHeader() {

    if(m_is_header_read == true)
        reset();
    
    // 移动到数据部分，即跳过头部
    file.getInputStream().seekg(m_head_size, std::ios::beg);
    m_is_header_read = true;
    return true;
}

int FileHeadReader::getHeadSize() {
    return m_head_size;
}

std::unordered_map<std::string, u64> FileHeadReader::getHeader() {
    return header;
}

size_t FileHeadReader::getFileSize() {
    return file.getFileSize();
}


std::unordered_map<std::string, u64> FileHeadReader::readCompleteHeader() {
        
    if(m_is_header_read == true){
       reset();
       m_is_header_read = false;
    }

    std::unordered_map<std::string, u64> headerValues;
    
    // 重置到文件开始
    file.getInputStream().seekg(0, std::ios::beg);
    
    // 根据文件格式读取各个字段
    for (const auto& format : file_format_list) {
        if (format.suffix_name == filename.substr(filename.find_last_of('.'))) {
            for (size_t i = 0; i < format.field_count; ++i) {
                const auto& field = format.fields[i];
                
                if (field.size > 0) {
                    // 根据字段大小读取相应字节数
                    switch (field.size) {
                        case 1:
                            headerValues[std::string(field.name)] = readu8();
                            break;
                        case 2:
                            headerValues[std::string(field.name)] = readu16();
                            break;
                        case 4:
                            headerValues[std::string(field.name)] = readu32();
                            break;
                        case 8:
                            headerValues[std::string(field.name)] = readu64();
                            break;
                        default:
                            // 对于其他大小，逐字节读取
                            u64 value = 0;
                            for (int j = 0; j < field.size; ++j) {
                                value |= (static_cast<u64>(readu8()) << (j * 8));
                            }
                            headerValues[std::string(field.name)] = value;
                            break;
                    }
                } else if (field.size == -1) {
                    // -1表示变长字段，如数据部分，不在此处读取
                    headerValues[std::string(field.name)] = 0;
                }
            }
            break;
        }
    }
    m_is_header_read = true;

    return headerValues;
}

void FileHeadReader::reset()
{
    m_is_header_read  = false;
    // 重置到文件开始
    file.getInputStream().seekg(0, std::ios::beg);
}

size_t FileHeadReader::tell() {    return static_cast<size_t>(file.getInputStream().tellg());}
