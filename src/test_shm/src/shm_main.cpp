//
// Created by edy on 24-1-22.
//
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <thread>
#include "test_shm/shm_common.hpp"


int main() {
    std::thread t_w([]() {
        std::unique_ptr<ShmCommon> shm_write = std::make_unique<ShmCommon>(sizeof(struct stuShareMemory),
                                                                           "/home/edy/vehicle/files/shm");
        auto shm_flag = shm_write->createShmWithoutSemaphore();
        if (shm_flag == nullptr) {
            return 0;
        }
        for (;;) {
            if (shm_flag->signal_i ==0) {
                std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
                char timestamp[80];
                strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
//                std::string filename =std::string(timestamp);
                memcpy(shm_flag->chBuffer,timestamp,strlen(timestamp));
                shm_flag->signal_i=1;
            }
        }
    });

    std::thread t_r([](){
        std::unique_ptr<ShmCommon> shm_write = std::make_unique<ShmCommon>(sizeof(struct stuShareMemory),
                                                                           "/home/edy/vehicle/files/shm");
        auto shm_flag = shm_write->createShmWithoutSemaphore();
        if (shm_flag == nullptr) {
            return 0;
        }
        for(;;){
            if(shm_flag->signal_i==1){
                printf("共享内存中的数据是%s\n",shm_flag->chBuffer);
                shm_flag->signal_i=0;
            }else{
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
    });

    t_w.join();
    t_r.join();


    return 0;
}