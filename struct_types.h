//
// Created by haydn on 2021-04-13.
//

#ifndef ASSINGMENT3PARTB_STRUCT_TYPES_H
#define ASSINGMENT3PARTB_STRUCT_TYPES_H

struct db_item{
    char acc_num[5];
    char pin[3];
    float funds;
};

struct messages {
    long int message_type;
    char *msg_type;
    struct db_item contents;
};


#endif //ASSINGMENT3PARTB_STRUCT_TYPES_H
