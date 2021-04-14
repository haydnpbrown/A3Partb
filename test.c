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
            //temp_item.acc_num = token;
            token = strtok(NULL, ",");
            strcpy(temp_item.pin, token);
            //temp_item.pin = token;
            token = strtok(NULL, ",");
            temp_item.funds = atof(token);
            found = 1;
            break;
        }
    }

    if (found != 1){
        strcpy(temp_item.acc_num, "0");
        //temp_item.acc_num = "0";
    }
    return temp_item;
}

int main(){

    struct db_item db;
    strcpy(db.acc_num, "00001");

    db = getItem(db.acc_num);
    printf("acc num: %s \n", db.acc_num);
    printf("pin: %s \n", db.pin);
    printf("funds: %f", db.funds);

}