//
// Created by chase on 2021-04-25.
//
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
#include "semfuncs.h"

#define MAX 256

/*
 * This function is used to update an item in the database. It is
 * used by the calculateInterest method so that the available funds in the account can
 * be updated. It takes a database item struct as a parameter which should contain
 * the account number to update and the new funds of the account.
 * The function works by copying the lines that aren't of interest into a new file, and replacing
 * the line of interest with the new line in the new file. Once this is done, the original file is
 * deleted and the new file is renamed to "db.txt"
 */
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
            if (strcmp(token, itemToReplace.acc_num) == 0){
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
 * This function is used to calculate the interest for each user in the system. after it calculated the
 * interest for the user it calls the replaceItem method to put the new information into the database.
 * This method is called in the main().
*/
void calculateInterest(){
    printf("Calculating interest for all accounts\n");
    FILE *dbfile;
    char str[100];
    char *filename = "db.txt";
    dbfile = fopen(filename, "r");
    if (dbfile == NULL) {
        printf("error opening the db file for reading \n");
        exit(1);
    }

    struct db_item temp_item;
    while (!feof(dbfile)) {
        strcpy(str, "\0");
        fgets(str, MAX, dbfile);
        str[strcspn(str, "\n")] = 0;
        char *token = strtok(str, ",");
        if (token != NULL) {
            int semid = semget((key_t) atoi(token), 1, 0666 | IPC_CREAT);
            if (!sp(semid)){
                exit(EXIT_FAILURE);
            }
            strcpy(temp_item.acc_num, token);
            token = strtok(NULL, ",");
            strcpy(temp_item.pin, token);
            token = strtok(NULL, ",");
            temp_item.funds = atof(token);
            printf("Initial Account: %s %s %.2f\n", temp_item.acc_num, temp_item.pin, temp_item.funds);
            if (temp_item.funds > 0.0) {
                float newB = temp_item.funds + (temp_item.funds * 0.01);
                temp_item.funds = newB;
            } else {
                float newB = temp_item.funds + (temp_item.funds * 0.02);
                temp_item.funds = newB;
            }
            printf("Account After Interest: %s %s %.2f\n", temp_item.acc_num, temp_item.pin, temp_item.funds);
            replaceItem(temp_item);
            if (!sv(semid)){
                exit(EXIT_FAILURE);
            }
            sleep(2);
        }
    }
    fclose(dbfile);
}
/*
 * The main loops for ever and every 60s it calls the method calculateInterest to calculate the interest
 * for every user in the db.
 */
int main(){
    while(1) {
        sleep(60);
        calculateInterest();
    }
}