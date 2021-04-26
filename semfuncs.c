#include <sys/sem.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "semun.h"

//union semun {
//    int val;
//    struct semid_ds *buf;
//    unsigned short *array;
//    struct seminfo *__buf;
//};

int set_semvalue(int semid){
    union semun sem_union;

    sem_union.val = 1;
    if (semctl(semid, 0, SETVAL, sem_union) == -1){
        return 0;
    }
    return 1;
}

void del_semvalue(int semid){
    union semun sem_union;

    if (semctl(semid, 0, IPC_RMID, sem_union) == -1){
        printf("Error deleting semaphore");
    }
}

int sp(int semid){
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(semid, &sem_b, 1) == -1){
        fprintf(stderr, "sp failed");
        return 0;
    }
    return 1;
}

int sv(int semid){
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;
    sem_b.sem_flg = SEM_UNDO;
    if (semop(semid, &sem_b, 1) == -1){
        fprintf(stderr, "sv failed");
        return 0;
    }
    return 1;
}
