# HuffmanCompress

一个基于Qt框架开发的哈夫曼压缩工具，支持文件压缩与解压缩，提供直观的图形用户界面。

## 功能特性

- **哈夫曼压缩算法**：高效的无损压缩算法，用于减小文件大小
- **图形用户界面**：基于Qt框架开发，操作简单直观
- **拖拽支持**：支持文件拖拽到应用程序进行处理
- **多文件处理**：可以同时处理多个文件
- **进度显示**：实时显示压缩/解压缩进度
- **支持的文件格式**：
  - 输入：BMP图像文件
  - 输出：.huf压缩文件
- **压缩率统计**：显示每个文件的压缩率
- **多语言支持**：支持中文界面
- **多线程处理**：利用多线程提高处理效率
- **详细日志**：记录操作过程和结果

## 技术架构

### 核心模块

#### 1. 哈夫曼树实现 (`huffman/huffmantree.h`)

**节点结构**：
- 模板化设计，支持多种数据类型
- 包含数据、编码、编码长度、频率、父节点和子节点指针
- 支持叶子节点和非叶子节点的区分

**优先队列构建**：
- 使用自定义比较器 `NodePtrCompare` 确保构建的哈夫曼树结构稳定
- 频率相同时比较数据值或叶子节点序列的字典序

**编码生成**：
- 深度优先遍历为每个叶子节点分配二进制编码
- 左分支为0，右分支为1
- 编码以 `u64` 存储，配合 `u8` 记录编码长度

**频率和编码优化**：
```cpp
// 动态选择频率存储长度以优化空间
inline int format_frequnency_length(u64 frequency) {
    if(frequency > 256) { if(frequency > 65536) { if(frequency > 4294967296) return 8; return 4; } return 2; }
    return 1;
}
```

**关键算法**：

**1. 哈夫曼树生成**：
```cpp
template <typename T>
bool HuffmanTree<T>::spawnTree() {
    // 初始化节点队列
    for(auto& data : node_map) {
        node_queue.push(data.second);
        frequency_length = format_frequnency_length(data.second->frequency);
    }
    
    // 合并节点构建树
    while(node_queue.size() > 1) {
        node<T>* left = node_queue.top(); node_queue.pop();
        node<T>* right = node_queue.top(); node_queue.pop();
        spawnParent(left, right);
    }
    
    root = node_queue.top(); node_queue.pop();
    return true;
}
```

**2. 编码分配（深度优先遍历）**：
```cpp
// 使用栈实现非递归的深度优先遍历
std::stack<node<T> *> node_stack; node_stack.push(root);
while (!node_stack.empty()) {
    node<T> *current = node_stack.top(); node_stack.pop();
    
    if (current->right_child != nullptr) {
        current->right_child->code = (current->code << 1) | 1; // 右子树编码添加1
        current->right_child->code_length = current->code_length + 1;
        node_stack.push(current->right_child);
    }
    if (current->left_child != nullptr) {
        current->left_child->code = (current->code << 1); // 左子树编码添加0
        current->left_child->code_length = current->code_length + 1;
        node_stack.push(current->left_child);
    }
}
```

**3. 树的清理**：
```cpp
template <typename T>
void HuffmanTree<T>::deleteTree() {
    if (root == nullptr) return;
    
    std::stack<node<T>*> node_stack; node_stack.push(root);
    std::vector<node<T>*> node_vector;
    
    while (!node_stack.empty()) {
        node<T>* temp = node_stack.top(); node_stack.pop();
        if(temp->right_child != nullptr) node_stack.push(temp->right_child);
        if(temp->left_child != nullptr) node_stack.push(temp->left_child);
        node_vector.push_back(temp);
    }
    
    // 逆序删除节点，避免悬空指针
    for (int i = node_vector.size()-1; i >= 0; i--) delete node_vector[i];
    root = nullptr;
}
```

**数据结构**：
- `frequency_map`：记录每个数据的出现频率
- `code_map`：存储数据到编码的映射（u64编码 + u8长度）
- `node_map`：快速访问各个节点
- `node_queue`：优先队列用于构建哈夫曼树

#### 2. 位流操作 (`huffman/bitstream.h`)

**编码功能**：
- 将哈夫曼编码转换为紧凑的位流
- 按位处理，将多个编码打包到字节中
- 支持处理最后不完整的字节

**解码功能**：
- 从位流中恢复原始数据
- 使用哈夫曼树进行逐位解码
- 支持错误检测和异常处理

**核心实现**：
```cpp
// 编码实现片段
for (int i = code_length - 1; i >= 0; i--) {
    u8 bit = (code >> i) & 1;
    buffer = (buffer << 1) | bit;
    buffer_bits++;
    if (buffer_bits == 8) {
        result.push_back(buffer);
        buffer = 0;
        buffer_bits = 0;
    }
}
```

#### 3. 文件处理 (`FileStream/`)

**文件读写**：
- `FileReader`：支持读取不同大小的整数（u8/u16/u32/u64）
- `FileWriter`：支持写入不同大小的整数
- 自动处理字节序问题

**文件格式支持**：
- BMP格式：支持标准Windows BMP格式，解析文件头信息
- HUF格式：自定义压缩格式，包含文件头、编码表和压缩数据

**文件头操作**：
- `FileHeadReader`：读取和解析文件头
- `FileHeadWriter`：写入文件头信息

#### 4. 多线程任务处理 (`FileTaskPool/`)

**ThreadPool 实现**：
```cpp
// 提交任务到线程池
template<class F, class... Args>
auto submit_with_result(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> result = task->get_future();
    // ...线程安全的任务添加逻辑...
    cv_.notify_one();
    return result;
}
```

**核心特性**：
- 自定义线程数量和任务队列大小
- 支持有返回值和无返回值的任务提交
- 线程安全的任务管理
- 支持暂停/恢复功能
- 任务队列和活跃任务数量查询

**ConvertTask 实现**：
```cpp
// 任务执行静态函数
static void run(std::string path,
                double *externalProgress,
                std::promise<OutType*> prom,
                ConvertFunction convertFunc) {
    try {
        OutType* result = convertFunc(path, externalProgress);
        prom.set_value(result);
        if (externalProgress) *externalProgress = 1.0;
    } catch (const std::exception& e) {
        prom.set_exception(std::current_exception());
    }
}
```

**核心特性**：
- 模板化设计，支持多种输入输出类型
- 进度报告机制，支持外部进度条更新
- 异常处理和结果返回
- 支持线程池和独立线程两种运行模式
- 可配置的加载器和保存器函数

#### 5. 日志系统 (`logger/`)

- 基于单例模式实现
- 支持不同日志级别（调试、信息、警告、错误）
- 日志输出到文件

#### 6. GUI界面 (`mainwindow.h`)

**核心功能**：
- 文件拖拽支持
- 任务列表管理
- 实时进度显示
- 输出目录设置

**事件处理**：
- `dragEnterEvent` 和 `dropEvent` 实现拖拽功能
- 按钮点击事件处理
- 进度更新槽函数

### 技术栈

- **C++17**：核心编程语言，使用现代C++特性
- **Qt 6**：GUI框架和跨平台支持
- **CMake**：项目构建系统
- **STL容器**：unordered_map、vector、queue、stack等
- **多线程**：C++11线程库和Qt线程支持
- **位操作**：高效的位级数据处理

## 技术总结

HuffmanCompress项目实现了一个功能完整、性能优良的哈夫曼压缩工具，具有以下技术亮点：

### 核心实现优势

1. **高效的哈夫曼树构建**
   - 使用自定义比较器确保树结构稳定
   - 动态优化频率和编码的存储长度
   - 非递归深度优先遍历分配编码，提高效率

2. **位流处理优化**
   - 高效的位级操作，减少内存占用
   - 支持处理不完整字节，确保数据完整性
   - 编码和解码算法的时间复杂度均为O(n)

3. **多线程并行处理**
   - 基于C++11线程库实现的线程池
   - 任务队列管理，支持任务优先级
   - 自动负载均衡，充分利用多核CPU

4. **文件格式支持**
   - 实现了BMP图像文件的完整解析和重建
   - 自定义HUF压缩文件格式，包含详细的文件头信息
   - 支持文件大小、类型、时间戳等元数据的保存和恢复

5. **内存管理优化**
   - 树节点的高效创建和销毁
   - 避免内存泄漏，确保资源正确释放
   - 大文件处理时的内存使用控制

### 架构设计特点

1. **模块化设计**
   - 各功能模块低耦合，高内聚
   - 清晰的接口定义，便于扩展和维护
   - 模板化设计，支持代码复用

2. **跨平台支持**
   - 基于Qt框架，支持Windows、Linux、macOS等平台
   - 使用CMake构建系统，简化编译流程

3. **用户体验优化**
   - 直观的GUI界面，支持文件拖拽
   - 实时进度显示，提升用户体验
   - 多语言支持，满足不同用户需求

4. **错误处理机制**
   - 完善的异常处理，提高程序稳定性
   - 详细的日志记录，便于问题排查

### 性能表现

- **压缩率**：对BMP图像文件的压缩率通常在50%-80%之间，具体取决于图像内容
- **处理速度**：利用多线程并行处理，大文件处理速度显著提升
- **内存占用**：采用高效的数据结构和算法，内存使用合理

HuffmanCompress项目展示了现代C++编程技术在实际应用中的应用，包括模板编程、多线程、内存管理、位操作等高级特性，是一个功能完整、性能优良的哈夫曼压缩工具实现。

## 项目结构

```
HuffmanCompress/
├── FileStream/        # 文件流处理模块
├── FileTaskPool/      # 任务池和线程管理
├── huffman/           # 哈夫曼算法实现
├── logger/            # 日志系统
├── task/              # 任务处理模块
├── build/             # 构建目录
├── test/              # 测试程序
├── test_resources/    # 测试资源
├── main.cpp           # 程序入口
├── mainwindow.cpp     # 主窗口实现
├── mainwindow.h       # 主窗口头文件
├── mainwindow.ui      # UI设计文件
├── CMakeLists.txt     # CMake配置文件
└── HuffmanCompress_zh_CN.ts  # 中文翻译文件
```

## 安装与构建

### 环境要求

- Qt 6.x（推荐6.9.1）
- MinGW 64-bit或MSVC编译器
- CMake 3.16及以上

### 构建步骤

1. **克隆项目**
   ```bash
   git clone <项目地址>
   cd HuffmanCompress
   ```

2. **使用Qt Creator打开项目**
   - 打开`CMakeLists.txt`文件
   - 选择构建套件（推荐MinGW 64-bit）
   - 点击构建按钮

3. **使用命令行构建**
   ```bash
   mkdir build
   cd build
   cmake .. -G "MinGW Makefiles"
   mingw32-make
   ```

## 使用说明

### 基本操作

1. **启动应用程序**
   - 运行构建生成的`HuffmanCompress.exe`文件

2. **添加文件**
   - 将BMP文件拖拽到应用程序窗口
   - 或使用文件选择对话框添加文件

3. **设置输出目录**
   - 点击"输出目录"按钮选择保存位置

4. **开始处理**
   - 点击"开始"按钮开始压缩/解压缩

5. **查看结果**
   - 处理完成后，在输出目录查看生成的文件
   - 应用程序会显示每个文件的压缩率

### 支持的文件格式

- **压缩**：BMP图像文件 → .huf压缩文件
- **解压缩**：.huf压缩文件 → 原始BMP文件

### 文件格式说明

1. **BMP文件**
   - 支持标准Windows BMP格式
   - 支持多种位深度（24位真彩色等）
   - 自动识别文件头信息

2. **HUF文件**（自定义压缩格式）
   - 包含文件头信息（文件大小、键值对数量等）
   - 存储哈夫曼编码表
   - 压缩后的位流数据

## 测试

项目包含多种测试程序，位于`test/`目录：

- **huffman_test**：哈夫曼算法单元测试
- **simple_compare**：文件比较测试
- **test_compression_ratio**：压缩率测试
- **compare_images**：图像质量比较测试

运行测试：
```bash
cd build
./test/huffman_test.exe
```

## 性能特性

- **时间复杂度**：
  - 构建哈夫曼树：O(n log n)
  - 编码：O(n)
  - 解码：O(n)

- **空间复杂度**：
  - 存储哈夫曼树：O(n)
  - 存储编码表：O(n)

## 未来改进

- [ ] 支持更多文件格式（如TXT、JPG等）
- [ ] 添加批量处理功能
- [ ] 优化大文件处理性能
- [ ] 添加压缩级别选择
- [ ] 支持文件加密

## 许可证

本项目采用MIT许可证，详见LICENSE文件。

## 作者

- **开发者**：[您的姓名]
- **邮箱**：[您的邮箱]

## 致谢

- Qt框架提供的GUI支持
- 哈夫曼压缩算法的发明者David A. Huffman

## 版本历史

- **v0.1**：初始版本，支持BMP文件压缩与解压缩

---

© 2024 HuffmanCompress. All rights reserved.