#include <iostream>
#include <fstream>
#include <cstdint>

// BMP文件头结构
#pragma pack(push, 1)
struct BMPHeader {
    uint16_t type;          // 文件类型，必须为'BM'
    uint32_t size;          // 文件大小
    uint16_t reserved1;     // 保留字
    uint16_t reserved2;     // 保留字
    uint32_t offset;        // 从文件头到像素数据的偏移量
    uint32_t dib_size;      // DIB头大小
    int32_t width;          // 图像宽度
    int32_t height;         // 图像高度
    uint16_t planes;        // 色板数量
    uint16_t bit_count;     // 每像素位数
    uint32_t compression;   // 压缩类型
    uint32_t image_size;    // 图像大小
    int32_t x_ppm;          // 水平分辨率
    int32_t y_ppm;          // 垂直分辨率
    uint32_t clr_used;      // 使用的颜色数
    uint32_t clr_important; // 重要颜色数
};
#pragma pack(pop)

// 创建一个简单的24位BMP文件
bool createTestBMP(const std::string& filename, int width, int height) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "无法创建文件: " << filename << std::endl;
        return false;
    }

    // 计算图像大小（每行字节数必须是4的倍数）
    int row_size = ((width * 3) + 3) & ~3;
    int image_size = row_size * height;
    
    // 填充BMP文件头
    BMPHeader header = {
        0x4D42,                      // 'BM' 类型
        static_cast<uint32_t>(sizeof(BMPHeader) + image_size), // 文件大小
        0, 0,                        // 保留字
        sizeof(BMPHeader),           // 偏移量
        40,                          // DIB头大小
        width, height,               // 宽高
        1,                           // 色板数量
        24,                          // 24位彩色
        0,                           // 不压缩
        static_cast<uint32_t>(image_size), // 图像大小
        0, 0,                        // 分辨率
        0, 0                         // 颜色数
    };

    // 写入文件头
    file.write(reinterpret_cast<const char*>(&header), sizeof(BMPHeader));
    
    // 创建简单的图像数据（渐变）
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // 创建RGB颜色（蓝色到红色的渐变）
            uint8_t blue = static_cast<uint8_t>((x * 255) / width);
            uint8_t red = static_cast<uint8_t>((y * 255) / height);
            uint8_t green = 128;
            
            // BMP文件中颜色顺序是BGR
            file.write(reinterpret_cast<const char*>(&blue), 1);
            file.write(reinterpret_cast<const char*>(&green), 1);
            file.write(reinterpret_cast<const char*>(&red), 1);
        }
        
        // 填充每行剩余的字节，确保是4的倍数
        int padding = row_size - (width * 3);
        for (int p = 0; p < padding; ++p) {
            file.write("\0", 1);
        }
    }
    
    file.close();
    std::cout << "成功创建测试BMP文件: " << filename << std::endl;
    std::cout << "图像尺寸: " << width << "x" << height << std::endl;
    std::cout << "文件大小: " << sizeof(BMPHeader) + image_size << " 字节\n";
    return true;
}

int main() {
    std::string filename = "test.bmp";
    int width = 100;
    int height = 100;
    
    if (createTestBMP(filename, width, height)) {
        std::cout << "测试BMP文件创建成功！" << std::endl;
        return 0;
    } else {
        std::cerr << "测试BMP文件创建失败！" << std::endl;
        return 1;
    }
}