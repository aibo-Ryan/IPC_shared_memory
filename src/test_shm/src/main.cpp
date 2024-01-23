#include <shared_mutex>
#include <iostream>
#include <thread>
#include "thread_safe_queue.cpp"

std::mutex mtx;
int shared_data = 0;

void use_unique() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "lock success" << std::endl;
    shared_data++;
    lock.unlock();
}

void use_owns_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    shared_data++;
    if (lock.owns_lock()) {
        std::cout << "owns lock" << std::endl;

    } else {
        std::cout << "not owns lock" << std::endl;
    }
    std::thread t([]() {
        std::unique_lock<std::mutex> lock_thread(mtx, std::defer_lock);
        if (lock_thread.owns_lock()) {
            std::cout << "thread owns lock" << std::endl;

        } else {
            std::cout << "thread not owns lock" << std::endl;

        }
        lock_thread.lock(); //程序会卡在这里，死锁，因为lock_thread无法去锁住mtx这个互斥量。它正被主线程的lock锁锁住了。
        if (lock_thread.owns_lock()) {
            std::cout << "thread owns lock" << std::endl;

        } else {
            std::cout << "thread not owns lock" << std::endl;

        }
        lock_thread.unlock();
    });
    t.join();
}

std::condition_variable cvA;
std::condition_variable cvB;
int num = 1;

void reliableImpl() {
    std::thread t1([]() {
        for (;;) {
            std::unique_lock<std::mutex> lock(mtx);
            //方式一与方式二是一个意思
//            while (num!=1){ //为什么用while不用if，因为有可能被系统给唤醒，用if就直接往下走了（不满足条件地走，是不对的）
//                cvA.wait(lock);
//            }
            //方式二
            cvA.wait(lock, []() {
                return num == 1;
            });
            num++;
            std::cout << "thread A print 1 ..." << std::endl;
            cvB.notify_one();
        }
    });

    std::thread t2([]() {
        for (;;) {
            std::unique_lock<std::mutex> lock(mtx);
            cvB.wait(lock, []() {
                return num == 2;
            });
            num--   ;
            std::cout << "thread B print 2 ..." << std::endl;
            cvA.notify_one();
        }
    });

    t1.join();

    t2.join();


}

void test_safe_queue(){
    ThreadSafeQueue<int> safe_queue;
    std::mutex mutex_print;
    std::thread producer([&](){
        for(int i=0;;i++){
            safe_queue.push(i);
            {
                std::lock_guard<std::mutex> lkprint(mutex_print);
                std::cout<<"produce i:"<<i<<std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    std::thread consumer([&](){
        for(;;){
            auto data=safe_queue.wait_and_pop();
            {
                std::lock_guard<std::mutex> lkprint(mutex_print);
                std::cout<<"consumer "<<*data<<std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    std::thread consumer2([&](){
        for(;;){
            auto data=safe_queue.try_pop();
            if(data!= nullptr)
            {
                std::lock_guard<std::mutex> lkprint(mutex_print);
                std::cout<<"consumer2 "<<*data<<std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    producer.join();
    consumer.join();
    consumer2.join();
}
int main() {
//    use_owns_lock();
//
    test_safe_queue();
    return 0;
}
