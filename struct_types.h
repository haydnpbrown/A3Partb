//
// Created by haydn on 2021-04-13.
//

#ifndef ASSINGMENT3PARTB_STRUCT_TYPES_H
#define ASSINGMENT3PARTB_STRUCT_TYPES_H

enum messagetype {PIN, BALANCE, WITHDRAW, UPDATE_DB, PIN_WRONG, OK};

struct db_item{
    char acc_num[6];
    char pin[4];
    float funds;
};

struct messages {
    long int message_type;
    enum messagetype msg_type;
    struct db_item contents;
};


#endif //ASSINGMENT3PARTB_STRUCT_TYPES_H
