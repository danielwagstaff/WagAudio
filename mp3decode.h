#ifndef MP3DECODE_H
#define MP3DECODE_H 

#include <mpg123.h>
#include <sndfile.h>

//PROTOTYPES
void*  decode(void*);
void cleanup(mpg123_handle *mh);

#endif
