//
// Created by chase on 2021-04-13.
//

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include "struct_types.h"

/*
 * The main() executes the interface between the user and the system by inputting the information given
 * by the user allowing the user to perform operations on there account. All the information gets sent
 * to the DB server to perform the functionality and echo back the result of the requested operation.
 */
int main(int argc, char *argv[]){

    int msgqidsend; //id of the outgoing msg queue
    int msgqidrec; //id of incoming message
    struct db_item current_acc; //acc info of the acc currently in use
    struct messages msg;
    //convert the program arg to the atm id
    long int id;
    char *eptr;
    id = strtol(argv[1], &eptr, 10);
    printf("ATM with id: %ld has started. \n", id);

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
            printf("\nThe Pin Entered is incorrect, Please Re-enter your information. \n");
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

        msg.message_type = id;
        msg.contents = current_acc;
        msg.msg_type = PIN;

        //send message to server
        if(msgsnd(msgqidsend, (void *)&msg, sizeof(struct messages), 0) == -1){
            printf("error sending message");
            exit(EXIT_FAILURE);
        }

        //receive message from server
        if (msgrcv(msgqidrec, (void *)&msg, sizeof(struct messages), id, 0) == -1){
            printf("error receiving pin\n");
            exit(1);
        }

        if (msg.msg_type == ACCOUNT_LOCKED){
            printf("\nAccount has been locked \n\n");
        }

        if(msg.msg_type == OK) {
            printf("Enter Banking Operations: \n");
            printf("1 for Balance check | Enter 2 for Withdrawal | Enter 3 for Money Transfer \n");
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
            }else if (op == 3){
                printf("Enter Account to Send Money to:\n");
                fgets(current_acc.transfer_acc_num, 15, stdin);
                current_acc.transfer_acc_num[strcspn(current_acc.transfer_acc_num, "\n")] = 0;

                printf("Enter Amount to send:\n");
                char transfer[100];
                fgets(transfer, 10, stdin);
                transfer[strcspn(transfer, "\n")] = 0;

                msg.msg_type = TRANSFER;
                current_acc.funds = (float) strtod(transfer, NULL);
                msg.contents = current_acc;
            }

            //Send message to server
            if (msgsnd(msgqidsend, (void *) &msg, sizeof(struct messages), 0) == -1) {
                printf("error sending message");
                exit(EXIT_FAILURE);
            }

            //receive message from server
            if (msgrcv(msgqidrec, (void *) &msg, sizeof(struct messages), id, 0) == -1) {
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
            else if(msg.msg_type == TRANSFER){
                printf("Account Balance after Transfer: %.2f\n\n", msg.contents.funds);
            }
        }
    }
}


