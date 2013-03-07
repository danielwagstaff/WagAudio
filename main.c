
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>

#include "./pcm.h"
#include "./con_msg.h"
#include "./main.h"
#include "./mp3decode.h"

#define MAX_LINES 100

verbosity_t verbosity = VERBOSE_OFF;

void display_playlist(char * plist);

int main(int argc, char * argv[])
{
    typedef enum
    {
        YES_PLAYLIST = 0,
        NO_PLAYLIST = 1
    }PLAYLIST_t;
    
    char * filetype;
    char * filename;
    char * temp;
    char * read = "/mnt/usb/MUSIC/02.wav";
    char * plist = NULL;
    char newfile[MAX_LINES];
    int fd=0;
    int i=0;
    unsigned char * pdecoded = NULL;
    struct filepaths file_paths;
    pthread_t thread0;
    FILE * playlist = NULL;
    PLAYLIST_t PLAYLIST = NO_PLAYLIST;

    file_paths.filepath = "/mnt/usb/MUSIC/02.wav";
    file_paths.temppath = "/tmp/tempwav";

    //READ IN CMD LINE OPERATORS
    for(i=1; i<argc; i++)
    {
        if( strcmp(argv[i],"-v")==0 )
        {
            verbosity = VERBOSE_ON;
        }
        if( strcmp(argv[i], "-f")==0  )
        {
            file_paths.filepath=argv[i+1];
        }
        if( strcmp(argv[i], "-p")==0 )
        {
            PLAYLIST = YES_PLAYLIST;
            plist = argv[i+1];
            playlist = fopen(argv[i+1], "r");
        }
    }
    
    con_msg(MSG_INFO, "\n###############################\n"); 
    con_msg(MSG_INFO, "\n***** Welcome to WagAudio *****\n"); 
    con_msg(MSG_INFO, "\n###############################\n"); 

    display_playlist(plist);

    while(read != NULL)
    {
        if(PLAYLIST == YES_PLAYLIST)
        {
            memset(newfile, 0, MAX_LINES);
            read = fgets(newfile, (MAX_LINES-1), playlist);
            if(read == NULL)
            {
                con_msg(MSG_INFO, "\nEnd of Playlist");
                break;
            }

            /*fgets reads in new line char. must remove for determining filetype, etc*/
            for(i = 0; i < strlen(newfile); i++)
            {
                if(newfile[i] == '\n' || newfile[i] == '\r')
                {
                    newfile[i] = '\0';
                }
            }
            
            file_paths.filepath = newfile;
        }
        else
        {
            read = NULL;
        }

        //Determine file type
        filetype=(file_paths.filepath+(strlen(file_paths.filepath)-3));
        
        if( strcmp(filetype, "mp3") && strcmp(filetype, "wav") )
        {
            con_msg(MSG_BAD, "\nUnrecognised file type %s for %s", filetype, file_paths.filepath);
            continue;
        }
       
        //Determine filename
        if( strchr(file_paths.filepath, '/') != NULL )
        {
            filename=(file_paths.filepath+(strlen(file_paths.filepath)));
 
            do{
                temp=strchr(filename, '/');
                filename -= 1;
            }while(temp == NULL);
 
            filename=filename+2;
        }
        else
        {
            filename=file_paths.filepath;
        }

        //If mp3, decode
        if( !strcmp(filetype, "mp3")  )
        {
            pthread_create(&thread0, NULL, decode, &file_paths);
            sleep(1); //decoder head-start
            //decode(&file_paths);
            file_paths.filepath = file_paths.temppath;
        }

        fd=open(file_paths.filepath, O_RDONLY);
        if(fd < 0)
        {
            con_msg(MSG_BAD, "\nFailed to Open File %s", file_paths.filepath);
            continue;
        }
        con_msg(MSG_GOOD, "Playing %s %s", filetype, filename);

        //Play wav or decoded mp3
        pcm_config();
        pcm_setup_and_play(fd);
        
        //Close file and free decoded memory
        if( close(fd) != 0 )
        {
            con_msg(MSG_BAD, "\nError Closing %s", file_paths.filepath);
        }
        else
        {
            con_msg(MSG_VERBOSE, "\nClosed %s", file_paths.filepath);
        }
        pthread_join(thread0, NULL);
    }

    fclose(playlist);
    con_msg(MSG_INFO, "\n");
    return 0;
}

void display_playlist(char * plist)
{
    char buffer[MAX_LINES];
    char * read = NULL;
    FILE * playlist = NULL;
    char * filename = NULL;
    char * temp = NULL;

    playlist = fopen(plist, "r");

    con_msg(MSG_INFO, "\nPlaying:\n");

    memset(buffer, 0, MAX_LINES);
    do
    {
        read = fgets(buffer, (MAX_LINES-1), playlist);
        if(read == NULL)
        {
            break;
        }

        //Determine filename
        if( strchr(read, '/') != NULL )
        {
            filename=(read+(strlen(read)));
 
            do{
                temp=strchr(filename, '/');
                filename -= 1;
            }while(temp == NULL);
 
            filename=filename+2;
        }
        else
        {
            filename=read;
        }

        con_msg(MSG_INFO, "%s", filename);

    }while(read != NULL);

   con_msg(MSG_INFO, "\n");

    fclose(playlist);
}
