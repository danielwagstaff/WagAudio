#ifndef CON_MSG_H
#define CON_MSG_H

//DEFINITIONS
#define DEFAULT "\033[1;0m"
#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define DGREY "\033[1;30m"

//ENUMERATIONS
typedef enum e_msg_type
{
    MSG_GOOD = 0,
    MSG_BAD = 1,
    MSG_INFO = 2,
    MSG_VERBOSE = 3
} msg_type_t;

//PROTOTYPE
int con_msg(msg_type_t msg, char * message, ...);

#endif
