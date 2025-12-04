#ifndef STACK_H
#define STACK_H
#include <stdexcept>

template <typename T>
struct node_s{
    T data;
    node_s* next;

    node_s(){
        next = nullptr;
    };
    node_s(T data): T(data){
        next = nullptr;
    };
    node_s(T data,node_s* ne):T(data),next(ne){};
    ~node_s(){
        delete next;
    };
};


template <typename T>
class stack{

    node_s<T>* top;
private:
    int count;

public:
    stack(): top(), count(0) {}

    ~stack(){
        while(top){
            pop();
        }
        top = nullptr;
    }

    void push(T data){
        node_s<T>* temp = new node_s<T>();
        temp->data = data;
        temp->next = top;
        top = temp;
        count++;
    }

    T pop(){
        if(top == nullptr){

            throw std::runtime_error("Stack is empty");
        }
        node_s<T>* temp = top;
        top = top->next;
        T data = temp->data;
        count--;
        delete temp;
        return data;
    }

    int size(){
        return count;
    }


};


#endif // STACK_H
