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

#define MAX 256

/*
 * append an account entry to the end of the database given a db_item
 */
void appendItem(struct db_item itemToAppend){
    FILE *dbfile;
    char *filename = "db.txt";
    dbfile = fopen(filename, "a");
    if(dbfile == NULL){
        printf("error opening the db file for appending \n");
        exit(1);
    }
    printf("The entry to append: %s,%s,%.2f \n", itemToAppend.acc_num, itemToAppend.pin, itemToAppend.funds);
    fprintf(dbfile, "\n%s,%s,%.2f", itemToAppend.acc_num, itemToAppend.pin, itemToAppend.funds);
    fclose(dbfile);
}

/*
 * return a db_item object given an account number to search for
 */
struct db_item getItem(char *acc){
    int found = 0;
    FILE *dbfile;
    char str[100];
    char *filename = "db.txt";
    dbfile = fopen(filename, "r");
    if(dbfile == NULL){
        printf("error opening the db file for reading \n");
        exit(1);
    }
    struct db_item temp_item;
    while(fgets(str, 100, dbfile) != NULL){
        char *token = strtok(str, ",");
        if (strcmp(acc, token) == 0){
            //this entry matches the acc number
            strcpy(temp_item.acc_num, token);
            token = strtok(NULL, ",");
            strcpy(temp_item.pin, token);
            token = strtok(NULL, ",");
            temp_item.funds = atof(token);
            found = 1;
            break;
        }
    }

    if (found != 1){
        strcpy(temp_item.acc_num, "0");
    }
    return temp_item;
}

void replaceItem(struct db_item itemToReplace){
    FILE *dbfile;
    FILE *dbfile2;
    char str[MAX];
    char str2[MAX];
    char *filename = "db.txt";
    char *tempname = "temp.txt";
    dbfile = fopen(filename, "r");
    if(dbfile == NULL){
        printf("error opening the db file for reading \n");
        exit(1);
    }
    dbfile2 = fopen(tempname, "w");
    if(dbfile2 == NULL){
        printf("error opening the db file for writing \n");
        exit(1);
    }

    while(!feof(dbfile)){
        strcpy(str, "\0");
        strcpy(str2, "\0");
        fgets(str, MAX, dbfile);
        str[strcspn(str, "\n")] = 0;
        strcpy(str2, str);
        char *token = strtok(str, ",");
        if (token != NULL){
            printf("the line to check: %s", str2);
            printf("the token to check: %s", token);
            if (strcmp(token, itemToReplace.acc_num) == 0){
                //this is the line to replace
                fprintf(dbfile2, "%s,%s,%f\n", itemToReplace.acc_num, itemToReplace.pin, itemToReplace.funds);
            } else {
                //copy the line from old to new file
                fprintf(dbfile2, "%s\n", str2);
            }
        }
    }

    fclose(dbfile);
    fclose(dbfile2);
    remove(filename);
    rename(tempname, filename);
}


void lockAccount(char *acc){

}

int main() {

    struct db_item db[100]; //array that represents the database
    int running = 1; //to determine if the system should continue to run
    int inmsgq; //id of the incoming msg queue
    int outmsgq; //id of the outgoing msg queue
    struct db_item current_acc; //acc info of the acc currently in use
    struct db_item db_acc; //the temp acc retrived from db used for comparison
    struct messages current_msg; //current message coming in/going out
    int pin_count = 0; //tracks the number of consecutive wrong pin entries
    pid_t pid;

    //delete the msg queue is it exists
    if (msgctl(msgget((key_t)1111, 0666 | IPC_CREAT), IPC_RMID, 0) == -1){
        printf("error closing the msg queue \n");
        exit(EXIT_FAILURE);
    }

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

    //wait for messages forever
    while(running){
        if (msgrcv(inmsgq, (void *)&current_msg, sizeof(struct messages), 0, 0) == -1){
            printf("error receiving message in db server \n");
            exit(EXIT_FAILURE);
        }
        current_acc = current_msg.contents;
        printf("receiving msg \n");
        printf("db item received: %s,%s,%f\n", current_acc.acc_num, current_acc.pin, current_acc.funds);

        if (current_msg.msg_type == PIN){
            //search file for acc num
            /*
             * acc # found
             * ->pin correct - return OK, save info locally
             * ->pin incorrect - if 3rd try, lock acc. if <3 tries, ++tries and return pin_wrong
             */
            //acc # not found - return pin_wrong
            db_acc = getItem(current_acc.acc_num);
            if (strcmp(db_acc.acc_num, "0") == 0){
                //the account number is not in the db, return PIN_WRONG
                current_msg.msg_type = PIN_WRONG;
                current_msg.message_type = 1;
                //send msg to ATM, message_type = 1
                if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                    printf("error sending msg to atm");
                    exit(EXIT_FAILURE);
                }
            } else {
                //the account is in the database, check its pin for correctness
                int temp_encode = atoi(current_acc.pin) - 1;
                char temp[4];
                sprintf(temp, "%d", temp_encode);
                if (strcmp(db_acc.pin, temp) == 0){
                    //the pins match, return a success message
                    current_msg.msg_type = OK;
                    current_msg.message_type = 1;
                    //send msg to ATM, message_type = 1
                    if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                        printf("error sending msg to atm");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    //the pins dont match, remove 1 try.
                    if (pin_count == 3){
                        //lock the account by setting the first char in the acc # to an x


                    } else {
                        //increment the # of tries and return a failure message
                        pin_count++;
                        current_msg.msg_type = PIN_WRONG;
                        current_msg.message_type = 1;
                        //send msg to ATM, message_type = 1
                        if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                            printf("error sending msg to atm");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
            }
        } else if (current_msg.msg_type == BALANCE){
            /*
             * this is a BALANCE request from the atm. The currently accessed acc from the db should be stored in
             * current_acc. Return the funds field from here to the ATM
             */
            current_msg.msg_type = BALANCE;
            current_msg.message_type = 1;
            current_msg.contents = db_acc; //the currently accessed account is already in db_acc
            if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                printf("error sending msg to atm");
                exit(EXIT_FAILURE);
            }
        } else if (current_msg.msg_type == WITHDRAW){
            /*
             * WITHDRAW request from the ATM. The current account is stored locally, so check its funds to see
             * if the request amount can be withdrawn. If it can, do so. If the account does not have enough funds
             * for the operation then notify the ATM.
             */
            if ((db_acc.funds - current_acc.funds) >= 0){
                current_acc.funds = db_acc.funds - current_acc.funds;
                replaceItem(current_acc);
                current_msg.msg_type = FUNDS_OK;
                current_msg.message_type = 1;
                current_msg.contents = current_acc;
                if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                    printf("error sending msg to atm");
                    exit(EXIT_FAILURE);
                }
            } else {
                //unable to withdraw the specified amount
                current_msg.msg_type = NSF;
                current_msg.message_type = 1;
                current_msg.contents = current_acc;
                if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                    printf("error sending msg to atm");
                    exit(EXIT_FAILURE);
                }
            }
        } else if (current_msg.msg_type == UPDATE_DB){
            /*
             * This is an UPDATE_DB request from the db editor. We first need to encrypt the user pin before
             * appending the request to the database.
             */
            printf("update db request being handled \n");
            int temp_encode = atoi(current_acc.pin) + 1; //encode the pin number
            char temp[4];
            sprintf(temp, "%d", temp_encode); //cast pin back to chars
            strcpy(current_acc.pin, temp);
            appendItem(current_acc); //append the db_item object to the database
        }
    }

    return 0;
}
