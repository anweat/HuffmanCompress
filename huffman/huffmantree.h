#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H
#include <unordered_map>
#include <vector>
#include <queue>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <stack>

// 前向声明
class bmpHandler;
class hufHandler;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

template <typename T>
struct node
{
    T data;
    u64 code;  
    u8 code_length;
    u64 frequency;
    node *parent;
    node *left_child;
    node *right_child;
    bool is_leaf;

    

    node(T data, u64 i_frequency) : data(data), code(0), code_length(0), frequency(i_frequency), is_leaf(true), parent(nullptr), left_child(nullptr), right_child(nullptr){}//含参含频率构造，用于解码

    node(T data): node(data,(u64)0){};//含参构造，用于记录

    node(int frequency,bool is_leaf):  data(0), code(0), code_length(0), frequency(frequency), is_leaf(is_leaf), parent(nullptr), left_child(nullptr), right_child(nullptr) {}//频率构造，用于非叶子节点
    node():node((int)0,false){};//默认空构造
    //实现优先队列，频率低的优先级高
    bool operator<(const node& other) const {

        if (frequency == other.frequency){
            return data > other.data;
        }

        return frequency > other.frequency;
    }

    ~node() {};
};

template <typename T>
struct NodePtrCompare
{
    // 辅助函数：使用栈遍历树，获取叶子节点的有序列表
    std::vector<T> getLeafNodes(const node<T>* root) const {
        std::vector<T> leaves;
        if (root == nullptr) return leaves;
        
        std::stack<const node<T>*> node_stack;
        node_stack.push(root);
        
        while (!node_stack.empty()) {
            const node<T>* current = node_stack.top();
            node_stack.pop();
            
            if (current->is_leaf) {
                leaves.push_back(current->data);
            } else {
                // 先右后左，保证叶子节点按左到右顺序收集
                if (current->right_child != nullptr) {
                    node_stack.push(current->right_child);
                }
                if (current->left_child != nullptr) {
                    node_stack.push(current->left_child);
                }
            }
        }
        
        return leaves;
    }
      
      bool operator()(const node<T> *lhs, const node<T> *rhs) const
      {
          if (lhs->frequency != rhs->frequency)
          {
              return lhs->frequency > rhs->frequency;
          }
          // 如果都是叶子节点，比较data值
          if (lhs->is_leaf && rhs->is_leaf)
          {
              return lhs->data > rhs->data;
          }
          if (lhs->is_leaf)
          {
              return false;
          }
          if (rhs->is_leaf)
          {
              return true;
          }
          
          // 当都是非叶子节点时，比较它们的叶子节点序列
          std::vector<T> lhs_leaves = getLeafNodes(lhs);
          std::vector<T> rhs_leaves = getLeafNodes(rhs);
          
          // 比较两个叶子节点序列的字典序
          for (size_t i = 0; i < std::min(lhs_leaves.size(), rhs_leaves.size()); ++i) {
              if (lhs_leaves[i] != rhs_leaves[i]) {
                  return lhs_leaves[i] > rhs_leaves[i];
              }
          }
          
          // 如果一个是另一个的前缀，较短的排在前面
          return lhs_leaves.size() > rhs_leaves.size();
      }
};
template <typename T>
class HuffmanTree
{

public:

    HuffmanTree();

    // 析构函数
    ~HuffmanTree();

    // 输入单个数据并记录频率
    bool input_data(T data);

    bool input_data(std::unordered_map<T, u64> datas);

    // 总入口，生成哈夫曼树
    bool spawnTree();

    // 获取进度
    int getProgress();

    // 获取编码
    std::pair<u64, u8> getCode(T data);

    std::unordered_map<T, u64> get_frequency_map() const;

    std::unordered_map<T, std::pair<u64, u8>> get_code_map() const;

    u8 get_frequency_length();

    u8 get_code_length();

    node<T>* get_root();

    std::priority_queue<node<T> *, std::vector<node<T> *>, NodePtrCompare<T>> queue_copy() const;

private: 
    std::priority_queue<node<T>*,std::vector<node<T>*>,NodePtrCompare<T>> node_queue;
    std::unordered_map<T, node<T>*> node_map;//数据——节点的表，便于查询
    std::unordered_map<T, u64> frequency_map;//频数表，便于记录频数
    std::unordered_map<T,std::pair<u64, u8>> code_map;//编码表，便于记录编码
    u64  total_count;//queue中结点总数
    u64  handled_count_twice;//已处理结点总数
    node<T> *root;
    bool isUpdateRoot;

    bool is_from_decode = false;

    u8 frequency_length = 1;

    u8 code_lenth = 1;

    // 删除树
    void deleteTree();

    // 生成父节点
    inline void spawnParent(node<T> *left_child, node<T> *right_child);

};




#endif
