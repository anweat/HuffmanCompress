#ifndef FILEFORMAT_H
#define FILEFORMAT_H
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>

// 统一的类型定义，确保所有模块使用一致的数据类型
typedef uint8_t  u8;
typedef int8_t   s8;
typedef uint16_t u16;
typedef int16_t  s16;
typedef uint32_t u32;
typedef int32_t  s32;
typedef uint64_t u64;
typedef int64_t  s64;
typedef uint8_t  bit;

typedef std::unordered_map<std::string, u32> HeaderMap;

struct FileHeaderField {
    std::string_view name;
    int size;
};

struct FileFormat
{
    std::string_view suffix_name;
    const FileHeaderField* fields;
    size_t field_count;
};

// BMP格式定义
constexpr FileHeaderField bmp_fields[] = {
    {"bfType", 2},            // 位图文件类型 ('BM')
    {"bfSize", 4},            // 位图文件大小
    {"bfReserved1", 2},       // 保留字，必须为0
    {"bfReserved2", 2},       // 保留字，必须为0
    {"bfOffBits", 4},         // 从文件头到数据的偏移量
    {"biSize", 4},            // 信息头大小
    {"biWidth", 4},           // 图像宽度
    {"biHeight", 4},          // 图像高度
    {"biPlanes", 2},          // 色板数量
    {"biBitCount", 2},        // 每像素位数
    {"biCompression", 4},     // 压缩类型
    {"biSizeImage", 4},       // 图像大小
    {"biXPelsPerMeter", 4},   // 水平分辨率
    {"biYPelsPerMeter", 4},   // 垂直分辨率
    {"biClrUsed", 4},         // 使用的颜色数
    {"biClrImportant", 4},    // 重要颜色数
    {"data", -1}              // 图像数据
};

// HUF格式定义
constexpr FileHeaderField huf_fields[] = {
    {"hufType", 2},         // HUF文件类型 ('UF')
    {"fileSize", 4},        // 文件大小
    {"reserved", 4},        // 保留
    {"keySize", 1},         // 键大小（1-8）
    {"valueSize", 1},       // 值大小（1-8）
    {"keyNum", 4},          // 键数量
    {"bitNum", 8},             //位集对应像素数（原像素个数）
    {"bitsetSize", 8},        // 位集大小（位集字节量）
    {"key_values", -1},             // 键值表
    {"bitset", -1},           // 压缩数据

};

constexpr const FileFormat file_format_list[] = {
    {".bmp", bmp_fields, sizeof(bmp_fields)/sizeof(bmp_fields[0])},
    {".huf", huf_fields, sizeof(huf_fields)/sizeof(huf_fields[0])}
};

// 获取文件格式信息的函数
static inline const std::unordered_map<std::string, int> get_file_format(const std::string& filename) {
  // 查找文件扩展名
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos == std::string::npos) {
      return std::unordered_map<std::string, int>();
  }
  
  std::string suffix = filename.substr(dot_pos);
  
  // 查找匹配的文件格式
  for (const auto& file_format : file_format_list) {
    if (file_format.suffix_name == suffix) {
      // 创建映射，忽略无效条目（键为空或值为0的条目）
      std::unordered_map<std::string, int> format_map;
      for(size_t i = 0; i < file_format.field_count; ++i) {
          const auto& field = file_format.fields[i];
          if (!field.name.empty() && field.size != 0) {
              format_map[std::string(field.name)] = field.size;
          }
      }
      return format_map;
    }
  }
  
  return std::unordered_map<std::string, int>();
}

// 检查文件是否为支持的格式
static inline bool is_supported_format(const std::string& filename) {
  size_t dot_pos = filename.find_last_of('.');
  if (dot_pos == std::string::npos) {
      return false;
  }
  
  std::string suffix = filename.substr(dot_pos);
  
  for (const auto& file_format : file_format_list) {
    if (file_format.suffix_name == suffix) {
      return true;
    }
  }
  
  return false;
}

// 获取支持的文件扩展名列表
static inline std::vector<std::string> get_supported_extensions() {
  std::vector<std::string> extensions;
  for (const auto& format : file_format_list) {
    extensions.push_back(std::string(format.suffix_name));
  }
  return extensions;
}

#endif
