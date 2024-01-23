//
// Created by edy on 24-1-22.
//
#ifndef TEST_SHM_SHM_COMMON_HPP
#define TEST_SHM_SHM_COMMON_HPP
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <iostream>
#include <cstring>
#include <memory>
#define GRIDSIZE 160000
#define SHARE_MEMORY_BUFFER_LEN 1024

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


struct shm_use_struct{
    int size_x;
    int size_y;
    double origin_x;
    double origin_y;
    double resolution;
    unsigned char costmap[GRIDSIZE];
};

struct stuShareMemory{
    int signal_i;
    char chBuffer[SHARE_MEMORY_BUFFER_LEN];
    stuShareMemory(){
        signal_i=0;
        memset(chBuffer,0,SHARE_MEMORY_BUFFER_LEN);
    }
};

class ShmCommon{
private:
    int sem_id_;
    int shm_id_;
    int size_;
    std::shared_ptr<stuShareMemory> ptr_;
    std::string shm_dir_="/home/root/shm";

public:
    ShmCommon(int size, std::string shm_dir);
    ~ShmCommon();

    std::shared_ptr<stuShareMemory> createShmWithoutSemaphore();
    std::shared_ptr<stuShareMemory> createShmWithSemaphore();


private:
    int createShmid();
    int createSemid();
    int semaphoreP();
    int semaphoreV();
};
#endif //TEST_SHM_SHM_COMMON_HPP
