#include<stdio.h>
#include<stdarg.h>
#include<stdlib.h>

#include "./main.h"
#include "./con_msg.h"

int con_msg(msg_type_t msg, char * message, ...)
{
    const char * p;
    va_list argp;
    int i;
    char * s;
    
    switch(msg)
    {
    case MSG_GOOD:
        printf(GREEN);
        break;
    case MSG_BAD:
        printf(RED);
        break;
    case MSG_INFO:
        printf(DEFAULT);
        break;
    case MSG_VERBOSE:
        if(verbosity == VERBOSE_OFF)
        {
            return 0;
        }
        printf(DGREY);
        break;
    }
    
    va_start(argp, message);
    for(p=message; *p!='\0'; p++)
    {
        if(*p!='%')
            {
                putchar(*p);
                continue;
            }
        switch(*++p)
            {
            case 'c':
                i = va_arg(argp, int);
                printf("%c", i);
                break;
            case 'd':
                i = va_arg(argp, int);
                printf("%d", i);
                break;
            case 's':
                s = va_arg(argp, char *);
                printf("%s", s);
                break;
            case 'p':
                i = va_arg(argp, int);
                printf("%p", i);
                break;
            case 'x':
                i = va_arg(argp, int);
                printf("%s", s); 
                break;
            case '%':
                putchar('%');
                break;
            }
    }
    va_end(argp);

    printf(DEFAULT);

    return 0;
}
