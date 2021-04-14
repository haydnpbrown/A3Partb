//
// Created by chase on 2021-04-13.
//

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include "struct_types.h"

#define MAX_TEXT 100000

int main(){

    int msgqid; //id of the outgoing msg queue
    struct db_item current_acc; //acc info of the acc currently in use
    struct messages msg;
    char tempFunds[1000000];

    //create the message queue
    msgqid = msgget((key_t)1111, 0666 | IPC_CREAT);
    if (msgqid == -1){
        printf("error creating the message queue");
        exit(EXIT_FAILURE);
    }

    while(1){
        printf("Enter Account Number :\n");
        fgets(current_acc.acc_num, 10, stdin);

        printf("Enter desired pin :\n");
        fgets(current_acc.pin, 10, stdin);


        printf("Enter Available Funds :\n");
        fgets(tempFunds, 100, stdin);

        current_acc.funds = (float) strtod(tempFunds, NULL);

//        printf("Summary of entered values\n");
//        printf("Account #: %s\n", current_acc.acc_num);
//        printf("Pin: %s\n", current_acc.pin);
//        printf("Balance: %f\n", current_acc.funds);


        msg.message_type = 0;
        msg.msg_type = "UPDATE_DB";
        msg.contents = current_acc;

        if(msgsnd(msgqid, (void *)&msg, sizeof(struct messages), 0) == -1){
            printf("error sending message");
            exit(EXIT_FAILURE);
        }
    }
}

