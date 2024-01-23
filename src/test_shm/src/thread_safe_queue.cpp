//
// Created by edy on 24-1-19.
//

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mut;// mutable好处：读取数据时，即使是const也要修改mutex，所以要用mutable
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    ThreadSafeQueue() {}

    //拷贝构造: const的好处：1.接收常量类型的变量 2.没有实现移动构造的时候会调用拷贝构造
    ThreadSafeQueue(ThreadSafeQueue const &other) {
        std::lock_guard<std::mutex> lk(other.mut);
        data_queue = other.data_queue;
    }

    void push(T new_value) {
        std::lock_guard<std::mutex> lk(mut);
        data_queue.push(new_value);
        data_cond.notify_one();
    }

    void wait_and_pop(T &value) {
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this]() {
            return !data_queue.empty();
        });
        value = data_queue.front();// 拷贝赋值，front内容被拷贝赋值给value，value就是引用的那个实参
        data_queue.pop();
    }

    // auto b=wait_and_pop(); // b=wait_and_pop() 计数为2，函数结束，计数减一，所以不会内存溢出
    std::shared_ptr<T> wait_and_pop() {
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [this]() {
            return !data_queue.empty();
        });
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));//初始化，计数为1
        data_queue.pop();
        return res;
    }

    //效率最高
    bool try_pop(T& value){
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return false;
        value=data_queue.front();
        data_queue.pop();
        return true;
    }
    //效率最高
    std::shared_ptr<T> try_pop(){
        std::lock_guard<std::mutex> lk(mut);
        if(data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
        data_queue.pop();
        return res;
    }

    bool empty()const{
        std::lock_guard<std::mutex> lk(mut);
        return data_queue.empty();
    }

};