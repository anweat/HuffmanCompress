#include "huffmantree.h"
#include <stack>

inline int format_frequnency_length(u64 frequency){
    if(frequency >= 256){
        if(frequency >= 65536){
            if(frequency >= 4294967296){
                return 8;
            }
            return 4;
        }
        return 2;
    }
    return 1;
}

inline int format_code_length(u64 code_length){
    if(code_length > 256){
        if(code_length > 65536){
            if(code_length > 4294967296){
                return 8;
            }
            return 4;
        }
        return 2;
    }
    return 1;
}


template <typename T>
HuffmanTree<T>::HuffmanTree() : isUpdateRoot(false), handled_count_twice(0){};

template <typename T>
HuffmanTree<T>::~HuffmanTree(){
    deleteTree();
    isUpdateRoot = false;
}

template <typename T>
bool HuffmanTree<T>::input_data(T data){
    if(node_map.find(data) == node_map.end()){
        node<T>* input = new node(data);
        input->frequency = 1;
        node_map.insert(std::make_pair(data, input));
        return true;
    }else{
        node_map[data]->frequency++;
        return true;
    }
}

template <typename T>
bool HuffmanTree<T>::input_data(std::unordered_map<T, u64> datas){
    for(auto& data : datas){
        node<T>* input = new node(data.first,data.second);
        node_queue.push(input);
        node_map[data.first] = input;
        frequency_map[data.first] = data.second;
        // 计算当前频率值所需的字节数
        u8 current_length = format_frequnency_length(data.second);
        // 更新frequency_length为最大值
        if(current_length > frequency_length) {
            frequency_length = current_length;
        }
    }
    is_from_decode = true;
    return true;
}

template <typename T>
bool HuffmanTree<T>::spawnTree(){
    isUpdateRoot  = true;
    if(is_from_decode == false){
        for(auto& data  : node_map){
            node_queue.push(data.second);
            // 计算当前频率值所需的字节数
            u8 current_length = format_frequnency_length(data.second->frequency);
            // 更新frequency_length为最大值
            if(current_length > frequency_length) {
                frequency_length = current_length;
            }
        }
    }

    // 计算节点数量
    total_count = node_map.size();
    
    // 确保code_lenth至少为1
    code_lenth = format_code_length(total_count);

    while(node_queue.size() > 1)
    {
        node<T>* left = node_queue.top();
        node_queue.pop();
        node<T>* right = node_queue.top();
        node_queue.pop();
        spawnParent(left,right);
        handled_count_twice++;
    }

    root = node_queue.top();
    node_queue.pop();

    if (root != nullptr)
    {
        root->code = 0;
        root->code_length = 0;
    }

    if (root != nullptr) {
        std::stack<node<T> *> node_stack;
        node_stack.push(root);
        
        while (!node_stack.empty()) {
            node<T> *current = node_stack.top();
            node_stack.pop();
            
            // 为左右子节点分配编码并压入栈中
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
            
            if (current->is_leaf == true) {
                handled_count_twice++;
            }
        }
    }

    for(auto& data : node_map){        
        frequency_map.insert(std::make_pair(data.first, data.second->frequency));        
        code_map.insert(std::make_pair(data.first, std::make_pair(data.second->code, data.second->code_length)));
    }

    return true;
}

template <typename T>
void HuffmanTree<T>::deleteTree(){
    if (root == nullptr) {
        return;
    }
    
    node<T> *temp = root;
    std::vector<node<T>*> node_vector;
    std::stack<node<T>*> node_stack;
    node_stack.push(root);
    while (!node_stack.empty()){
        temp = node_stack.top();
        node_stack.pop();
        if(temp->right_child != nullptr){
            node_stack.push(temp->right_child);
        }
        if (temp->left_child != nullptr)
        {
            node_stack.push(temp->left_child);
        }
        node_vector.push_back(temp);
    }

    for (int i = 0; i < node_vector.size(); i++){
        delete node_vector[i];
    }
    
    // 重置根节点
    root = nullptr;
}

template <typename T>
inline void HuffmanTree<T>::spawnParent(node<T>* left_child, node<T>* right_child){
    node<T>* parent = new node<T>(left_child->frequency + right_child->frequency,false);
    parent->left_child = left_child;
    parent->right_child = right_child;
    left_child->parent = right_child->parent = parent;
    node_queue.push(parent);
}

template <typename T>
std::pair<u64,u8> HuffmanTree<T>::getCode(T data){
    auto it = node_map.find(data);
    if (it != node_map.end())
    {
        return std::make_pair(it->second->code, it->second->code_length);
    }
    // 返回默认值，表示未找到
    return std::make_pair(0, 0);
}

template <typename T>
std::priority_queue<node<T> *, std::vector<node<T> *>, NodePtrCompare<T>> HuffmanTree<T>::queue_copy() const {
    // 直接使用priority_queue的复制构造函数
    return std::priority_queue<node<T> *, std::vector<node<T> *>, NodePtrCompare<T>>(node_queue);
}

template <typename T>
std::unordered_map<T, u64> HuffmanTree<T>::get_frequency_map() const {
    return frequency_map;
}

template <typename T>
int HuffmanTree<T>::getProgress(){
    return handled_count_twice/(total_count*2);
}

template <typename T>
u8 HuffmanTree<T>::get_code_length(){
    return code_lenth;
}

template <typename T>
u8 HuffmanTree<T>::get_frequency_length(){ 
    return frequency_length;
}

template <typename T>
std::unordered_map<T, std::pair<u64, u8>> HuffmanTree<T>::get_code_map() const {
    return code_map;
}

template <typename T>
node<T> *HuffmanTree<T>::get_root()
{
    return root;
}



template class HuffmanTree<unsigned char>;
template class HuffmanTree<unsigned short>;
template class HuffmanTree<unsigned int>;
