#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

// 读取文件内容到vector
vector<unsigned char> readFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "无法打开文件: " << filename << endl;
        return {};
    }
    
    // 获取文件大小
    file.seekg(0, ios::end);
    size_t size = file.tellg();
    file.seekg(0, ios::beg);
    
    // 读取文件内容
    vector<unsigned char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    
    return buffer;
}

int main() {
    // 文件路径
    string originalFile = "D:/Desktop/bmp/test1.bmp";
    string decompressedFile = "test1_decompressed.bmp";
    
    // 读取文件内容
    vector<unsigned char> originalData = readFile(originalFile);
    vector<unsigned char> decompressedData = readFile(decompressedFile);
    
    if (originalData.empty() || decompressedData.empty()) {
        return 1;
    }
    
    // 比较文件大小
    cout << "原始文件大小: " << originalData.size() << " 字节" << endl;
    cout << "解压缩文件大小: " << decompressedData.size() << " 字节" << endl;
    
    if (originalData.size() != decompressedData.size()) {
        cout << "文件大小不同！" << endl;
        
        // 找出大小差异点
        size_t minSize = min(originalData.size(), decompressedData.size());
        cout << "前 " << minSize << " 字节相同吗？" << endl;
        
        // 比较前minSize字节
        bool allSame = true;
        for (size_t i = 0; i < minSize; ++i) {
            if (originalData[i] != decompressedData[i]) {
                cout << "第 " << i << " 字节不同: 原始=0x" << hex << (int)originalData[i] 
                     << ", 解压缩=0x" << (int)decompressedData[i] << dec << endl;
                allSame = false;
                break; // 只显示第一个不同
            }
        }
        
        if (allSame) {
            cout << "前 " << minSize << " 字节完全相同" << endl;
        }
        
        // 显示文件末尾差异
        if (originalData.size() > minSize) {
            cout << "原始文件多出 " << (originalData.size() - minSize) << " 字节" << endl;
            cout << "原始文件末尾数据 (最后20字节):" << endl;
            for (size_t i = originalData.size() - 20; i < originalData.size(); ++i) {
                cout << hex << setw(2) << setfill('0') << (int)originalData[i] << " ";
            }
            cout << dec << endl;
        } else {
            cout << "解压缩文件多出 " << (decompressedData.size() - minSize) << " 字节" << endl;
            cout << "解压缩文件末尾数据 (最后20字节):" << endl;
            for (size_t i = decompressedData.size() - 20; i < decompressedData.size(); ++i) {
                cout << hex << setw(2) << setfill('0') << (int)decompressedData[i] << " ";
            }
            cout << dec << endl;
        }
    } else {
        // 文件大小相同，比较内容
        bool allSame = true;
        for (size_t i = 0; i < originalData.size(); ++i) {
            if (originalData[i] != decompressedData[i]) {
                cout << "第 " << i << " 字节不同: 原始=0x" << hex << (int)originalData[i] 
                     << ", 解压缩=0x" << (int)decompressedData[i] << dec << endl;
                allSame = false;
                break; // 只显示第一个不同
            }
        }
        
        if (allSame) {
            cout << "文件内容完全相同！" << endl;
        }
    }
    
    return 0;
}
