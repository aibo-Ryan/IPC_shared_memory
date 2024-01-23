//
// Created by edy on 24-1-22.
//
#include "test_shm/shm_common.hpp"

ShmCommon::ShmCommon(int size, std::string shm_dir):size_(size),shm_dir_(shm_dir) {

}

ShmCommon::~ShmCommon() {
    shmdt(ptr_.get());
    shmctl(shm_id_,IPC_RMID, nullptr);
}

std::shared_ptr<stuShareMemory> ShmCommon::createShmWithoutSemaphore() {
    int shm_id=createShmid();
    if(shm_id>0){
        stuShareMemory* ptr=(stuShareMemory* )shmat(shm_id, nullptr,0);
        if(ptr<(void *)0){
            printf("绑定共享内存首地址失败\n");
            return std::shared_ptr<stuShareMemory>();
        }else{
            std::shared_ptr<stuShareMemory> ptr_return(ptr);
            shm_id_=shm_id;
            ptr_=ptr_return;
            return ptr_return;
        }
    }
}

std::shared_ptr<stuShareMemory> ShmCommon::createShmWithSemaphore() {
    int shm_id=createShmid();
    if(shm_id>0){
        stuShareMemory* ptr=(stuShareMemory* )shmat(shm_id, nullptr,0);
        if(ptr<(void *)0){
            printf("绑定共享内存首地址失败\n");
            return std::shared_ptr<stuShareMemory>();
        }else{
            sem_id_=createSemid();
            if(sem_id_>0){

                std::shared_ptr<stuShareMemory> ptr_return(ptr);
                shm_id_=shm_id;
                ptr_=ptr_return;
                return ptr_return;
            }else{
                printf("信号量创建失败\n");
                return std::shared_ptr<stuShareMemory>();
            }

        }
    }
}

int ShmCommon::createShmid() {
    key_t key= ftok(shm_dir_.c_str(),1);
    if(key<0){
        perror("ftok");
        return -1;
    }
    printf("Shared memory key generated OK.\n");

    int shmid=shmget(key,size_,IPC_CREAT| IPC_EXCL | 0666);
    if(shmid<0){
        if(errno==EEXIST){
            shmid = shmget(key, size_, IPC_CREAT | 0666);

            if (shmid < 0)
            {
                printf("Shared memory has been created, but shmget() error: %s\n", strerror(errno));
                return -2;
            } else{
                printf("Shared memory has been created, shm_id =%d\n", shmid);
                return shmid;

            }


        }else{
            printf("Shared memory has not been created, error: %s\n", strerror(errno));
            return -3;

        }
    } else{
        printf("Shared memory is created, shm_id=%d\n", shmid);
        return shmid;
    }
}

int ShmCommon::createSemid() {
    key_t key= ftok(shm_dir_.c_str(),1);
    if(key<0){
        perror("ftok");
        return -1;
    }
    printf("Semaphore key generated OK.\n");

    int semid=semget(key,1,IPC_CREAT| IPC_EXCL | 0666);
    if(semid<0){
        if(errno==EEXIST){
            semid = semget(key, 1, IPC_CREAT | 0666);

            if (semid < 0)
            {
                printf("Semaphore has been created, but semget() error: %s\n", strerror(errno));
                return -2;
            } else{
                printf("Semaphore has been created, sem_id =%d\n", semid);
                return semid;
            }


        }else{
            printf("Semaphore has not been created, error: %s\n", strerror(errno));
            return -3;
        }
    } else{
        printf("Semaphore is created, sem_id=%d\n", semid);
        union semun sem_union;
        sem_union.val=1;
        if(semctl(semid,0,SETVAL,sem_union)==-1){
            return -4;
        }else{
            return semid;

        }
    }
}

int ShmCommon::semaphoreP() {
    struct sembuf sem_b;
    sem_b.sem_num=0;
    sem_b.sem_op=-1;
    sem_b.sem_flg=SEM_UNDO|IPC_NOWAIT;
    if(semop(sem_id_,&sem_b,1)==-1){
        printf("semaphore_p failed\n");
        return 0;
    }
    return 1;
}

int ShmCommon::semaphoreV() {

    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO | IPC_NOWAIT;

    if (semop(sem_id_, &sem_b, 1) == -1)
    {
        printf("semaphore_v failed\n");
        return 0;
    }
    return 1;
}
