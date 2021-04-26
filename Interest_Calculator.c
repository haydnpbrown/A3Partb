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

#define MAX 256


struct db_item getItem(int linenumber){
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
    int count = 1;
    while(fgets(str, 100, dbfile) != NULL){

        if (linenumber == count) {
            char *token = strtok(str, ",");
            strcpy(temp_item.acc_num, token);
            token = strtok(NULL, ",");
            strcpy(temp_item.pin, token);
            token = strtok(NULL, ",");
            temp_item.funds = atof(token);
            break;
        }
        count++;
    }

    return temp_item;
}

void replaceItem(struct db_item itemToReplace){
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
            //printf("the line to check: %s", str2);
            //printf("the token to check: %s", token);
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



int main(){
    while(1) {
        sleep(60);

        int i = 1;
        int running = 1;
        while (running == 1) {
            struct db_item account = getItem(i);
            if (account.acc_num[0] == '\0') {
                running = 0;
            } else {
                printf("Initial Account: %s %s %.2f\n", account.acc_num, account.pin, account.funds);

                if (account.funds > 0.0) {
                    float newB = account.funds + (account.funds * 0.01);
                    account.funds = newB;
                } else {
                    float newB = account.funds + (account.funds * 0.02);
                    account.funds = newB;
                }
                printf("Account After Interest: %s %s %.2f\n", account.acc_num, account.pin, account.funds);
                replaceItem(account);
                i++;
            }
            sleep(2);
        }
    }
}