//
// Created by chase on 2021-04-13.
//

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
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
        if (msg.msg_type == PIN_WRONG){
            printf("\n The Pin Entered is incorrect, Please Re-enter your information. \n");
        }

        printf("Enter Account Number :\n");
        fgets(current_acc.acc_num, 15, stdin);
        current_acc.acc_num[strcspn(current_acc.acc_num, "\n")] = 0;

        if(current_acc.acc_num == "X"){ //exit iff account number entered is X
            exit(1);
        }

        printf("Enter pin :\n");
        fgets(current_acc.pin, 15, stdin);
        current_acc.pin[strcspn(current_acc.pin, "\n")] = 0;

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


        //If pin is wrong (5 = PING_WRONG)
//       if(msg.msg_type == PIN_WRONG){
//           for(int i = 0; i < 3; i++){
//
//               printf("pin wrong %d re-enter pin", i + 1);
//
//               //re-enter pin
//               printf("Enter pin :\n");
//               fgets(current_acc.pin, 10, stdin);
//
//               msg.message_type = 1;
//               msg.contents = current_acc;
//               msg.msg_type = PIN;
//
//               //send pin again
//               if(msgsnd(msgqidsend, (void *)&msg, sizeof(struct messages), 0) == -1){
//                   printf("error sending message");
//                   exit(EXIT_FAILURE);
//               }
//
//               //receive message
//               if (msgrcv(msgqidrec, (void *)&msg, sizeof(struct messages), 0, 0) == -1){
//                   printf("error receiving pin\n");
//                   exit(1);
//               }
//
//               //break is pin is correct (6 = OK)
//               if (msg.msg_type == OK) {
//                   break;
//               }
//           }
//           //break is pin is correct (6 = OK)
//           if (msg.msg_type == OK) {
//               break;
//           }
//           //EXIT and block account because pin was wrong 3 times
//           else{
//               printf("account blocked");
//               exit(1);
//           }
//       }

        if(msg.msg_type == OK) {
            printf("Enter Banking Operations: \n");
            printf("1 for Balance check | Enter 2 for Withdrawal \n");
            printf(">> ");

            char operation[1];
            fgets(operation, 10, stdin);
            operation[strcspn(operation, "\n")] = 0;

            int op = atoi(operation);


            if(op == 1){
                msg.msg_type = BALANCE;
            }else if(op == 2){
                printf("Enter Amount to Withdraw:  \n");
                char withdraw[100];
                fgets(withdraw, 10, stdin);
                withdraw[strcspn(withdraw, "\n")] = 0;
                printf("Amount to withdraw %s\n", withdraw);

                msg.msg_type = WITHDRAW;
                current_acc.funds = (float) strtod(withdraw, NULL);
                msg.contents = current_acc;
            }


            //Send message to server
            if (msgsnd(msgqidsend, (void *) &msg, sizeof(struct messages), 0) == -1) {
                printf("error sending message");
                exit(EXIT_FAILURE);
            }

            //receive message from server
            if (msgrcv(msgqidrec, (void *) &msg, sizeof(struct messages), 0, 0) == -1) {
                printf("error receiving pin\n");
                exit(1);
            }

            if(msg.msg_type == BALANCE){
                printf("Account Balance: %.2f\n\n", msg.contents.funds);
            }else if(msg.msg_type == FUNDS_OK){
                printf("Account Balance after Withdrawal: %.2f\n\n", msg.contents.funds);
            }
            else if(msg.msg_type == NSF){
                printf("Insufficient Funds \n\n");
            }
        }
    }
}


