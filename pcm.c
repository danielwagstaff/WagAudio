
/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <errno.h>

#include "./pcm.h"
#include "./con_msg.h"
#include "./main.h"

int pcm_config()
{
    int val;

    con_msg(MSG_VERBOSE, "\n***** PCM TYPES AND FORMAT *****\n");

    con_msg(MSG_VERBOSE, "\nALSA Library Version: %s\n", SND_LIB_VERSION_STR);

    con_msg(MSG_VERBOSE, "\nPCM Stream Types:\n");
    for(val=0; val<= SND_PCM_STREAM_LAST; val++)
    {
        con_msg(MSG_VERBOSE, "  %s\n", snd_pcm_stream_name( (snd_pcm_stream_t)val) );
    }

    con_msg(MSG_VERBOSE, "\nPCM Formats:\n");
    for(val=0; val<= SND_PCM_FORMAT_LAST; val++)
    {
        if(snd_pcm_format_name( (snd_pcm_format_t)val ) != NULL)
        {
            con_msg(MSG_VERBOSE, "  %s (%s)\n", snd_pcm_format_name( (snd_pcm_stream_t)val ),
                                  snd_pcm_format_description( (snd_pcm_format_t)val )
                  );
        }
    }

    con_msg(MSG_VERBOSE, "\nPCM Sub-Formats:\n");
    for(val=0; val<= SND_PCM_SUBFORMAT_LAST; val++)
    {
        con_msg(MSG_VERBOSE, "  %s (%s)\n", snd_pcm_subformat_name( (snd_pcm_subformat_t)val ),
                              snd_pcm_subformat_description( (snd_pcm_subformat_t)val )
              );
    }

    con_msg(MSG_VERBOSE, "\nPCM States:\n");
    for(val=0; val<= SND_PCM_STATE_LAST; val++)
    {
        con_msg(MSG_VERBOSE, "  %s\n", snd_pcm_state_name( (snd_pcm_stream_t)val) );
    }


    return 0;
}

int pcm_setup_and_play(int fd)
{
    con_msg(MSG_VERBOSE, "\n\n***** PCM SETUP *****\n");

    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val, val2;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    int short_read_cnt = 0;
    int i = 0;

    /* Open PCM device for playback. */
    rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0) {
        con_msg(MSG_BAD, "unable to open pcm device: %s\n", snd_strerror(rc));
        exit(1);
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(handle, params);


    /* Set the desired hardware parameters. */

    /* Interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(handle, params, 2);

    /* 44100 bits/second sampling rate (CD quality) */
    val = 44100;
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0)
    {
      con_msg(MSG_BAD, "unable to set hw parameters: %s\n", snd_strerror(rc));
      exit(1);
    }

    /* Display information about the PCM interface */
    con_msg(MSG_VERBOSE, "PCM handle name = '%s'\n", snd_pcm_name(handle));

    con_msg(MSG_VERBOSE, "PCM state = %s\n", snd_pcm_state_name(snd_pcm_state(handle)));

    snd_pcm_hw_params_get_access(params, (snd_pcm_access_t *) &val);
    con_msg(MSG_VERBOSE, "access type = %s\n", snd_pcm_access_name((snd_pcm_access_t)val));

    snd_pcm_hw_params_get_format(params, &val);
    con_msg(MSG_VERBOSE, "format = '%s' (%s)\n", snd_pcm_format_name((snd_pcm_format_t)val),
                                                 snd_pcm_format_description((snd_pcm_format_t)val));

    snd_pcm_hw_params_get_subformat(params, (snd_pcm_subformat_t *)&val);
    con_msg(MSG_VERBOSE, "subformat = '%s' (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t)val),
                                                    snd_pcm_subformat_description((snd_pcm_subformat_t)val));

    snd_pcm_hw_params_get_channels(params, &val);
    con_msg(MSG_VERBOSE, "channels = %d\n", val);

    snd_pcm_hw_params_get_rate(params, &val, &dir);
    con_msg(MSG_VERBOSE, "rate = %d bps\n", val);

    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    con_msg(MSG_VERBOSE, "period time = %d us\n", val);

    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    con_msg(MSG_VERBOSE, "period size = %d frames\n", (int)frames);

    snd_pcm_hw_params_get_buffer_time(params, &val, &dir);
    con_msg(MSG_VERBOSE, "buffer time = %d us\n", val);

    snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t *) &val);
    con_msg(MSG_VERBOSE, "buffer size = %d frames\n", val);

    snd_pcm_hw_params_get_periods(params, &val, &dir);
    con_msg(MSG_VERBOSE, "periods per buffer = %d frames\n", val);

    snd_pcm_hw_params_get_rate_numden(params, &val, &val2);
    con_msg(MSG_VERBOSE, "exact rate = %d/%d bps\n", val, val2);

    val = snd_pcm_hw_params_get_sbits(params);
    con_msg(MSG_VERBOSE, "significant bits = %d\n", val);

    snd_pcm_hw_params_get_tick_time(params, &val, &dir);
    con_msg(MSG_VERBOSE, "tick time = %d us\n", val);

    val = snd_pcm_hw_params_is_batch(params);
    con_msg(MSG_VERBOSE, "is batch = %d\n", val);

    val = snd_pcm_hw_params_is_block_transfer(params);
    con_msg(MSG_VERBOSE, "is block transfer = %d\n", val);

    val = snd_pcm_hw_params_is_double(params);
    con_msg(MSG_VERBOSE, "is double = %d\n", val);

    val = snd_pcm_hw_params_is_half_duplex(params);
    con_msg(MSG_VERBOSE, "is half duplex = %d\n", val);

    val = snd_pcm_hw_params_is_joint_duplex(params);
    con_msg(MSG_VERBOSE, "is joint duplex = %d\n", val);

    val = snd_pcm_hw_params_can_overrange(params);
    con_msg(MSG_VERBOSE, "can overrange = %d\n", val);

    val = snd_pcm_hw_params_can_mmap_sample_resolution(params);
    con_msg(MSG_VERBOSE, "can mmap = %d\n", val);

    val = snd_pcm_hw_params_can_pause(params);
    con_msg(MSG_VERBOSE, "can pause = %d\n", val);

    val = snd_pcm_hw_params_can_resume(params);
    con_msg(MSG_VERBOSE, "can resume = %d\n", val);

    val = snd_pcm_hw_params_can_sync_start(params);
    con_msg(MSG_VERBOSE, "can sync start = %d\n", val);
  
  
    con_msg(MSG_VERBOSE, "***** PLAY FILE *****");
  
    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
    size = frames * 4;   //2 bytes/sample, 2 channels
    buffer = (char *)malloc(size);

    printf("\n"); //Required, else last print message not displayed.
        
    while(1)
    {
        rc = read(fd, buffer, size);
        if (rc == 0)
        {
            break;
        }
        else if(rc != size)
        {
            (short_read_cnt==0) ? con_msg(MSG_BAD, "") : con_msg(MSG_BAD, "short read: read %d bytes\n", rc);
            ++short_read_cnt;
        }
  
        rc = snd_pcm_writei(handle, buffer, frames);
        if (rc == -EPIPE)
        {   /* EPIPE means underrun */
            con_msg(MSG_BAD, "underrun occurred\n");
            snd_pcm_prepare(handle);
        }
        else if (rc < 0)
        {
            con_msg(MSG_BAD, "error from writei: %s\n", snd_strerror(rc));
        }
        else if (rc != (int)frames)
        {
            con_msg(MSG_BAD, "short write, write %d frames\n", rc);
        }
    }
  
    snd_pcm_drain(handle);
    snd_pcm_close(handle);
    free(buffer);

    return fd;
}
