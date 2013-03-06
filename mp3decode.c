
#include <stdio.h>
#include <mpg123.h>
#include <sndfile.h>
#include <strings.h>

#include "./mp3decode.h"
#include "./con_msg.h"
#include "./main.h"

void* decode(void * file_paths)
{
    mpg123_handle * mh;
    int status;
    const char ** pdecoders;
    int  channels = 0, encoding = 0;
    long rate = 0;
    SNDFILE* sndfile = NULL;
    SF_INFO sfinfo;
    unsigned char * buffer = NULL;
    size_t buffer_size = 0;
    size_t done = 0;
    off_t samples = 0;
    char * filepath = NULL;
    char * temppath = NULL;

    filepath = ((struct filepaths *)file_paths)->filepath;
    temppath = ((struct filepaths *)file_paths)->temppath;

    /*Initialise library - call only once per process*/
    status = mpg123_init();
    if(status != MPG123_OK)
    {
        cleanup(mh);
        return NULL;
    }
    
    mh = mpg123_new(NULL, NULL);
    if(mh == NULL)
    {
        cleanup(mh);
        return NULL;
    }

    if(verbosity == VERBOSE_ON)
    {
        con_msg(MSG_VERBOSE, "\nCurrent Decoder: %s", mpg123_current_decoder(mh) );
        con_msg(MSG_VERBOSE, "\nAvailable Decoders:\n");
        pdecoders = mpg123_decoders();
        while(*pdecoders != NULL)
        {
            con_msg(MSG_BAD, "%s\n", *pdecoders);
            pdecoders+=1;
        }
    }

    status = mpg123_open(mh, filepath);
    if(status != MPG123_OK)
    {
        con_msg(MSG_BAD, "\n Unable to Open mp3 %s", filepath);
        cleanup(mh);
        return NULL;
    }
    
    status = mpg123_getformat(mh, &rate, &channels, &encoding);
    if(status != MPG123_OK)
    {
        con_msg(MSG_BAD, "\nUnable to get format data for %s", filepath);
        cleanup(mh);
        return NULL;
    }
    con_msg(MSG_VERBOSE, "\nmh: %p, rate: %d, channels: %d, encoding: %d", mh, rate, channels, encoding);

    /*Ensure output format will not change*/
    mpg123_format_none(mh);
    mpg123_format(mh, rate, channels, encoding);

    bzero(&sfinfo, sizeof(sfinfo));
    sfinfo.samplerate = rate;
    sfinfo.channels = channels;
    sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_PCM_16;
    con_msg(MSG_VERBOSE, "Creating WAV with %i channels and %liHz.\n", channels, rate);

    sndfile = sf_open(temppath, SFM_WRITE, &sfinfo);
    if(sndfile == NULL)
    {
        con_msg(MSG_BAD, "Cannot Create SNDFILE");
        cleanup(mh);
        return NULL;
    }
    else
    {
        con_msg(MSG_VERBOSE, "SNDFILE: %d", sndfile);
    }

    buffer_size = mpg123_outblock(mh);
    buffer = malloc(buffer_size);

    do{
        sf_count_t more_samples;

        status = mpg123_read(mh, buffer, buffer_size, &done);
        more_samples = sf_write_short(sndfile, (short*)buffer, done/sizeof(short));

        samples += more_samples;
    }while(status == MPG123_OK);

    if(status != MPG123_DONE)
    {
        con_msg(MSG_BAD, "\nFile %s not fully decoded", filepath);
    }

    sf_close(sndfile);
    free(buffer);
    cleanup(mh);

    return NULL;
}

void cleanup(mpg123_handle *mh)
{
    mpg123_close(mh);
    mpg123_delete(mh);
    mpg123_exit();
}
