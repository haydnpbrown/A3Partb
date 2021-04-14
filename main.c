#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include "struct_types.h"




int main() {

    struct db_item db[100]; //array that represents the database
    int running = 1; //to determine if the system should continue to run
    int inmsgq; //id of the incoming msg queue
    int outmsgq; //id of the outgoing msg queue
    struct db_item current_acc; //acc info of the acc currently in use
    struct messages current_msg; //current message coming in/going out
    int pin_count = 0; //tracks the number of consecutive wrong pin entries
    pid_t pid;


    //create inbound msg queue
    inmsgq = msgget((key_t)1111, 0666 | IPC_CREAT);
    if (inmsgq == -1){
        printf("error creating the incoming msg queue \n");
        exit(EXIT_FAILURE);
    }

    //create outgoing msg queue
    outmsgq = msgget((key_t)1112, 0666 | IPC_CREAT);
    if (outmsgq == -1){
        printf("error creating the outgoing msg queue \n");
        exit(EXIT_FAILURE);
    }

    //fork here to create the db editor process

    //wait for messages forever
    while(running){
        if (msgrcv(inmsgq, (void *)&current_msg, sizeof(struct messages), 0, 0) == -1){
            printf("error receiving message in db server \n");
            exit(EXIT_FAILURE);
        }
        if (strcmp(current_msg.msg_type, "PIN") == 0){
            //search file for acc num
            /*
             * acc # found
             * ->pin correct - return OK, save info locally
             * ->pin incorrect - if 3rd try, lock acc. if <3 tries, ++tries and return pin_wrong
             */
            //acc # not found - return pin_wrong
        } else if (strcmp(current_msg.msg_type, "WITHDRAW") == 0){

        } else if (strcmp(current_msg.msg_type, "BALANCE") == 0){

        } else if (strcmp(current_msg.msg_type, "UPDATE_DB") == 0){

        }
    }

    return 0;
}
