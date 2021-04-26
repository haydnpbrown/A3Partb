//
// Created by haydn on 2021-04-13.
//

#ifndef ASSINGMENT3PARTB_STRUCT_TYPES_H
#define ASSINGMENT3PARTB_STRUCT_TYPES_H

enum messagetype {PIN, BALANCE, WITHDRAW, UPDATE_DB, PIN_WRONG, OK, ACCOUNT_LOCKED, NSF, FUNDS_OK, TRANSFER};

struct db_item{
    char acc_num[10];
    char pin[10];
    float funds;
    char transfer_acc_num[10];
};

struct messages {
    long int message_type;
    enum messagetype msg_type;
    struct db_item contents;
};

#endif //ASSINGMENT3PARTB_STRUCT_TYPES_H
