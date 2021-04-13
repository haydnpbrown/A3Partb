#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/sem.h>
#include "struct_types.h"

int main() {

    struct db_item db[100]; //array that represents the database
    int running = 1; //to determine if the system should continue to run
    int inmsgq; //id of the incoming msg queue
    int outmsgq; //id of the outgoing msg queue
    struct db_item current_acc; //acc info of the acc currently in use
    int pin_count = 0; //tracks the number of consecutive wrong pin entries


    return 0;
}
