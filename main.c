#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "struct_types.h"
#include "semfuncs.h"

#define MAX 256

/*
 * append an account entry to the end of the database given a db_item and create a semaphore
 * for the entry.
 */
void appendItem(struct db_item itemToAppend) {
    FILE *dbfile;
    char *filename = "db.txt";
    dbfile = fopen(filename, "a");
    if (dbfile == NULL) {
        printf("error opening the db file for appending \n");
        exit(1);
    }
    printf("The entry to append: %s,%s,%.2f \n", itemToAppend.acc_num, itemToAppend.pin, itemToAppend.funds);
    fprintf(dbfile, "%s,%s,%.2f\n", itemToAppend.acc_num, itemToAppend.pin, itemToAppend.funds);
    fclose(dbfile);

    //create semaphore for the new entry
    int semid = semget((key_t) atoi(itemToAppend.acc_num), 1, 0666 | IPC_CREAT);
    if (!set_semvalue(semid)) {
        printf("There was an error setting the sem value for %s semaphore\n", itemToAppend.acc_num);
        exit(EXIT_FAILURE);
    }
}

/*
 * return a db_item object from the database given an account number to search for
 */
struct db_item getItem(char *acc) {
    int found = 0;
    FILE *dbfile;
    char str[100];
    char *filename = "db.txt";
    dbfile = fopen(filename, "r");
    if (dbfile == NULL) {
        printf("error opening the db file for reading \n");
        exit(1);
    }
    struct db_item temp_item;
    while (fgets(str, 100, dbfile) != NULL) {
        char *token = strtok(str, ",");
        if (strcmp(acc, token) == 0) {
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

    if (found != 1) {
        //return a 0 for the account number to indicate to the calling function that it wasnt found
        strcpy(temp_item.acc_num, "0");
    }
    return temp_item;
}

/*
 * This function is used to update an item in the database. It is
 * used by the withdraw method so that the available funds in the account can
 * be updated. It takes a databse item struct as a parameter which should contain
 * the account number to update and the new funds of the account.
 * The function works by copying the lines that aren't of interest into a new file, and replacing
 * the line of interest with the new line in the new file. Once this is done, the original file is
 * deleted and the new file is renamed to "db.txt"
 */
void replaceItem(struct db_item itemToReplace) {
    //encode pin of itemToReplace
    int temp_encode = atoi(itemToReplace.pin) - 1;
    char temp[4];
    sprintf(temp, "%d", temp_encode);
    strcpy(itemToReplace.pin, temp);

    FILE *dbfile;
    FILE *dbfile2;
    char str[MAX];
    char str2[MAX];
    char *filename = "db.txt";
    char *tempname = "temp.txt";
    dbfile = fopen(filename, "r");
    if (dbfile == NULL) {
        printf("error opening the db file for reading \n");
        exit(1);
    }
    dbfile2 = fopen(tempname, "w");
    if (dbfile2 == NULL) {
        printf("error opening the db file for writing \n");
        exit(1);
    }

    while (!feof(dbfile)) {
        strcpy(str, "\0");
        strcpy(str2, "\0");
        fgets(str, MAX, dbfile);
        str[strcspn(str, "\n")] = 0;
        strcpy(str2, str);
        char *token = strtok(str, ",");
        if (token != NULL) {
            if (strcmp(token, itemToReplace.acc_num) == 0) {
                //this is the line to replace
                fprintf(dbfile2, "%s,%s,%.2f\n", itemToReplace.acc_num, itemToReplace.pin, itemToReplace.funds);
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


/*
 * This function is used to set the first character in an account number
 * to an "X" to indicate that the user's account is locked after 3 consecutive
 * failed logins.
 */
void lockAccount(struct db_item itemToLock) {
    struct db_item temp_item = getItem(itemToLock.acc_num);
    FILE *dbfile;
    FILE *dbfile2;
    char str[MAX];
    char str2[MAX];
    char *filename = "db.txt";
    char *tempname = "temp.txt";
    dbfile = fopen(filename, "r");
    if (dbfile == NULL) {
        printf("error opening the db file for reading \n");
        exit(1);
    }
    dbfile2 = fopen(tempname, "w");
    if (dbfile2 == NULL) {
        printf("error opening the db file for writing \n");
        exit(1);
    }

    while (!feof(dbfile)) {
        strcpy(str, "\0");
        strcpy(str2, "\0");
        fgets(str, MAX, dbfile);
        str[strcspn(str, "\n")] = 0;
        strcpy(str2, str);
        char *token = strtok(str, ",");
        if (token != NULL) {
            printf("the line to check: %s", str2);
            printf("the token to check: %s", token);
            if (strcmp(token, itemToLock.acc_num) == 0) {
                //this is the line to replace
                itemToLock.acc_num[0] = 'X';
                fprintf(dbfile2, "%s,%s,%.2f\n", itemToLock.acc_num, temp_item.pin, temp_item.funds);
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

/*
 * This function is used to initialize a semaphore for each
 * entry in the database when the db server starts
 */
void initializeSemaphores() {
    FILE *dbfile;
    char str[100];
    char *filename = "db.txt";
    dbfile = fopen(filename, "r");
    if (dbfile == NULL) {
        printf("error opening the db file for reading \n");
        exit(1);
    }

    while (!feof(dbfile)) {
        strcpy(str, "\0");
        fgets(str, MAX, dbfile);
        str[strcspn(str, "\n")] = 0;
        char *token = strtok(str, ",");
        if (token != NULL) {
            printf("the sem to create/get: %s", token);
            int semid = semget((key_t) atoi(token), 1, 0666 | IPC_CREAT);
            if (!set_semvalue(semid)) {
                printf("There was an error setting the sem value for %s semaphore\n", token);
                exit(EXIT_FAILURE);
            }
            printf("semaphore created for token %s with id %d \n", token, semid);
        }
    }
    fclose(dbfile);
}

int main() {
    struct db_item db[100]; //array that represents the database
    int running = 1; //to determine if the system should continue to run
    int inmsgq; //id of the incoming msg queue
    int outmsgq; //id of the outgoing msg queue
    struct db_item current_acc; //acc info of the acc currently in use
    struct db_item db_acc; //the temp acc retrived from db used for comparison
    struct db_item db_transfer_acc; //account from transfer
    struct messages current_msg; //current message coming in/going out
    int pin_count = 1; //tracks the number of consecutive wrong pin entries
    pid_t pid;

    //delete the inbound msg queue is it exists
    if (msgctl(msgget((key_t) 1111, 0666 | IPC_CREAT), IPC_RMID, 0) == -1) {
        printf("error closing the msg queue \n");
        exit(EXIT_FAILURE);
    }

    //delete the outgoing msg queue is it exists
    if (msgctl(msgget((key_t) 1112, 0666 | IPC_CREAT), IPC_RMID, 0) == -1) {
        printf("error closing the msg queue \n");
        exit(EXIT_FAILURE);
    }

    //create inbound msg queue
    inmsgq = msgget((key_t) 1111, 0666 | IPC_CREAT);
    if (inmsgq == -1) {
        printf("error creating the incoming msg queue \n");
        exit(EXIT_FAILURE);
    }

    //create outgoing msg queue
    outmsgq = msgget((key_t) 1112, 0666 | IPC_CREAT);
    if (outmsgq == -1) {
        printf("error creating the outgoing msg queue \n");
        exit(EXIT_FAILURE);
    }

    //initialize db semaphores
    printf("initialize semaphores\n");
    initializeSemaphores();

    //wait for messages forever
    while (running) {
        if (msgrcv(inmsgq, (void *) &current_msg, sizeof(struct messages), 0, 0) == -1) {
            printf("error receiving message in db server \n");
            exit(EXIT_FAILURE);
        }
        current_acc = current_msg.contents;

        if (current_msg.msg_type == PIN) {
            pid = fork();
            if (pid < 0) {
                printf("Error forking\n");
            } else if (pid == 0) {
                int semid = semget((key_t) atoi(current_acc.acc_num), 1, 0666 | IPC_CREAT);
                if (!sp(semid)){
                    exit(EXIT_FAILURE);
                }
                db_acc = getItem(current_acc.acc_num);
                if (strcmp(db_acc.acc_num, "0") == 0) {
                    //the account number is not in the db, return PIN_WRONG
                    current_msg.msg_type = PIN_WRONG;
                    if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                        printf("error sending msg to atm");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    //the account is in the database, check its pin for correctness
                    int temp_encode = atoi(current_acc.pin) - 1;
                    char temp[4];
                    sprintf(temp, "%d", temp_encode);
                    if (strcmp(db_acc.pin, temp) == 0) {
                        //the pins match, return a success message
                        current_msg.msg_type = OK;
                        if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                            printf("error sending msg to atm");
                            exit(EXIT_FAILURE);
                        }
                        pin_count = 0;
                    } else {
                        //the pins dont match, see if the account should be locked
                        if (pin_count == 3) {
                            //lock the account by setting the first char in the acc # to an x
                            lockAccount(current_acc);
                            current_msg.msg_type = ACCOUNT_LOCKED;
                            if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                                printf("error sending msg to atm");
                                exit(EXIT_FAILURE);
                            }
                            pin_count = 0;
                        } else {
                            //increment the # of tries and return a failure message
                            pin_count++;
                            current_msg.msg_type = PIN_WRONG;
                            if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                                printf("error sending msg to atm");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                }
                if (!sv(semid)){
                    exit(EXIT_FAILURE);
                }
            }
        } else if (current_msg.msg_type == BALANCE) {
            pid = fork();
            if (pid < 0) {
                printf("Error forking\n");
            } else if (pid == 0) {
                int semid = semget((key_t) atoi(current_acc.acc_num), 1, 0666 | IPC_CREAT);
                if (!sp(semid)){
                    exit(EXIT_FAILURE);
                }
                /*
                 * this is a BALANCE request from the ATM. Find the item sent from the ATM in the database
                 * and return it to the ATM so it can extract its funds.
                 */
                db_acc = getItem(current_acc.acc_num);
                current_msg.msg_type = BALANCE;
                current_msg.contents = db_acc;
                if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                    printf("error sending msg to atm");
                    exit(EXIT_FAILURE);
                }
                if (!sv(semid)){
                    exit(EXIT_FAILURE);
                }
            }
        } else if (current_msg.msg_type == WITHDRAW) {
            pid = fork();
            if (pid < 0) {
                printf("Error forking\n");
            } else if (pid == 0) {
                int semid = semget((key_t) atoi(current_acc.acc_num), 1, 0666 | IPC_CREAT);
                if (!sp(semid)){
                    exit(EXIT_FAILURE);
                }
                /*
                 * This is a WITHDRAW request from the ATM. Get the item sent from the ATM from the database
                 * and compare its funds field to the amount of funds request to be withdrawn. If the request doesn't
                 * bring the existing funds below 0, it is valid and the request can be fulfilled. If the request
                 * does bring the funds below 0 it is not completed and the message sent back to the ATM is NFS to
                 * indicate that the request cannot be performed.
                 */
                db_acc = getItem(current_acc.acc_num);
                if ((db_acc.funds - current_acc.funds) >= 0) {
                    current_acc.funds = db_acc.funds - current_acc.funds;
                    replaceItem(current_acc);
                    current_msg.msg_type = FUNDS_OK;
                    current_msg.contents = current_acc;
                    if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                        printf("error sending msg to atm");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    //unable to withdraw the specified amount
                    current_msg.msg_type = NSF;
                    current_msg.contents = current_acc;
                    if (msgsnd(outmsgq, (void *) &current_msg, sizeof(struct messages), 0) == -1) {
                        printf("error sending msg to atm");
                        exit(EXIT_FAILURE);
                    }
                }
                if (!sv(semid)){
                    exit(EXIT_FAILURE);
                }
            }
        } else if (current_msg.msg_type == UPDATE_DB) {
            pid = fork();
            if (pid < 0) {
                printf("Error forking\n");
            } else if (pid == 0){
                /*
             * This is an UPDATE_DB request from the db editor. We first need to encrypt the user pin before
             * appending the request to the database.
             */
                int temp_encode = atoi(current_acc.pin) - 1; //encode the pin number
                char temp[4];
                sprintf(temp, "%d", temp_encode); //cast pin back to chars
                strcpy(current_acc.pin, temp);
                appendItem(current_acc); //append the db_item object to the database
            }
        }
        else if (current_msg.msg_type == TRANSFER){
            pid = fork();
            if (pid < 0){
                printf("Error forking\n");
            } else if (pid == 0){
                /*
                 * this is a transfer funds request, which transfers some amount of money from one account
                 * to another. The money is taken from the current_acc.acc_num, which is the user accessing the ATM,
                 * and it is sent to the transfer_acc_num. This function allows a user to go below $0 in funds,
                 * although we could have implemented a check to prevent this. We decided to allow a user to go negative
                 * to make use of the functionality in the interestCalculator which calculates the interest of an
                 * account with funds below 0.
                 */
                int semid = semget((key_t) atoi(current_acc.acc_num), 1, 0666 | IPC_CREAT);
                int semid2 = semget((key_t) atoi(current_acc.transfer_acc_num), 1, 0666 | IPC_CREAT);
                if (!sp(semid)){
                    exit(EXIT_FAILURE);
                }
                if (!sp(semid2)){
                    exit(EXIT_FAILURE);
                }
                db_transfer_acc = getItem(current_acc.transfer_acc_num);
                db_acc = getItem(current_acc.acc_num);
                db_acc.funds = db_acc.funds - current_acc.funds;
                db_transfer_acc.funds = db_transfer_acc.funds + current_acc.funds;
                current_acc.funds = db_acc.funds;
                replaceItem(current_acc);
                replaceItem(db_transfer_acc);

                current_msg.msg_type = TRANSFER;
                current_msg.contents = current_acc;
                if (msgsnd(outmsgq, (void *)&current_msg, sizeof(struct messages), 0) == -1){
                    printf("error sending msg to atm");
                    exit(EXIT_FAILURE);
                }
                if (!sv(semid)){
                    exit(EXIT_FAILURE);
                }
                if (!sv(semid2)){
                    exit(EXIT_FAILURE);
                }
            }
        }
    }

    return 0;
}
