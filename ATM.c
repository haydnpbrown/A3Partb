//
// Created by chase on 2021-04-13.
//

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/msg.h>
#include "struct_types.h"

int main(){

    int msgqidsend; //id of the outgoing msg queue
    int msgqidrec; //id of incoming message
    struct db_item current_acc; //acc info of the acc currently in use
    struct messages msg;

    //initializing the message to send queue
    msgqidsend = msgget((key_t)1111, 0666 | IPC_CREAT);
    if (msgqidsend == -1){
        printf("error creating the message queue");
        exit(EXIT_FAILURE);
    }

    //initializing the message to receive queue
    msgqidrec = msgget((key_t)1112, 0666 | IPC_CREAT);
    if (msgqidrec == -1){
        printf("error creating the message queue");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("Enter Account Number :\n");
        fgets(current_acc.acc_num, 10, stdin);

        if(current_acc.acc_num == "X"){ //exit iff account number entered is X
            exit(1);
        }

        printf("Enter pin :\n");
        fgets(current_acc.pin, 10, stdin);

        msg.message_type = 1;
        msg.contents = current_acc;
        msg.msg_type = PIN;

        //send message to server
        if(msgsnd(msgqidsend, (void *)&msg, sizeof(struct messages), 0) == -1){
            printf("error sending message");
            exit(EXIT_FAILURE);
        }

        //receive message from server
        if (msgrcv(msgqidrec, (void *)&msg, sizeof(struct messages), 0, 0) == -1){
            printf("error receiving pin\n");
            exit(1);
        }

        printf()

        //If pin is wrong (5 = PING_WRONG)
       if(msg.msg_type == PIN_WRONG){
           for(int i = 0; i < 3; i++){

               printf("pin wrong %d re-enter pin", i + 1);

               //re-enter pin
               printf("Enter pin :\n");
               fgets(current_acc.pin, 10, stdin);

               msg.message_type = 1;
               msg.contents = current_acc;
               msg.msg_type = PIN;

               //send pin again
               if(msgsnd(msgqidsend, (void *)&msg, sizeof(struct messages), 0) == -1){
                   printf("error sending message");
                   exit(EXIT_FAILURE);
               }

               //receive message
               if (msgrcv(msgqidrec, (void *)&msg, sizeof(struct messages), 0, 0) == -1){
                   printf("error receiving pin\n");
                   exit(1);
               }

               //break is pin is correct (6 = OK)
               if (msg.msg_type == OK) {
                   break;
               }
           }
           //break is pin is correct (6 = OK)
           if (msg.msg_type == OK) {
               break;
           }
           //EXIT and block account because pin was wrong 3 times
           else{
               printf("account blocked");
               exit(1);
           }
       }

        printf("Enter Banking Operations (BALANCE): \n");
        char temp[10];
        fgets(temp, 10, stdin);
        msg.msg_type = BALANCE;

        //Send message to server
        if(msgsnd(msgqidsend, (void *)&msg, sizeof(struct messages), 0) == -1){
            printf("error sending message");
            exit(EXIT_FAILURE);
        }

        //receive message from server
        if (msgrcv(msgqidrec, (void *)&msg, sizeof(struct messages), 0, 0) == -1){
            printf("error receiving pin\n");
            exit(1);
        }

        printf("Account Balance: %f", msg.contents.funds);

        // DIDNT IMPLEMENT WITHDRAW, SORRY :/
    }
}


