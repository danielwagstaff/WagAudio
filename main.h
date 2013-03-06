#ifndef MAIN_H
#define MAIN_H


//ENUMERATIONS
typedef enum e_verbosity
{
    VERBOSE_ON = 0,
    VERBOSE_OFF = 1,
} verbosity_t;

extern verbosity_t verbosity;

//STRUCTURES
struct filepaths {
    char * filepath;
    char * temppath;
};

#endif
