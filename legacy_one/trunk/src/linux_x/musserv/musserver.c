/*************************************************************************
 *  musserver.c
 *
 *  Copyright (C) 1995-1997 Michael Heasley (mheasley@hmc.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
  // isdigit
#include "musserver.h"

#if defined(SCOUW2)
#include "usleep.h"
#endif


// Globals
byte verbose = 0;
byte changevol_allowed = 1;
byte parent_check = 1;  // check parent process
byte no_devices_exit = 0;
char parent_proc[32];  // parent process /proc/num

int qid;  // IPC message queue id

music_data_t  music_data;

// Locals
static FILE *infile;

static byte sel_dvt = DVT_DEFAULT;  // dev_e
static int dev_port_num = -1;
static int dev_type = -1;  // as per the ioctl listing

#define  TIMEOUT_UNIT_MS  200
#define  DEFAULT_TIMEOUT_SEC  300
// Timeout in seconds.
static unsigned int timeout = DEFAULT_TIMEOUT_SEC;


void show_help(void)
{
    printf("musserver version %s\n", MUS_VERSION);
    printf("Usage: musserver [options]\n\n");
    printf("  -l               List detected music devices and exit\n");
    printf("  -d <dev> <port>  Device preference.\n" );
    printf("    <dev>: M = Any Midi      E = Ext Midi\n" );
    printf("           L = FluidSynth    T = TiMidity\n" );
    printf("           S = Any Synth     F = FM Synth\n" );
#ifdef AWE32_SYNTH_SUPPORT
    printf("           A = Awe32 Synth\n" );
#endif
    printf("    <port>         Optional, defaults to first found. \n");
    printf("  -u <number>      Use device of type <number> where <number> is the type\n");
    printf("                   reported by 'musserver -l'.\n");
    printf("  -V <vol>         Ignore volume change messages from Doom\n");
    printf("  -c               Do not check whether the parent process is alive\n");
    printf("  -t <number>      Timeout for getting IPC message queue (sec).\n");
    printf("  -v -v2 -v3       Verbose. Default level 1.\n");
    printf("  -x               Exit if no devices found.\n");
    printf("  -h               Help: print this message and exit\n");
}

// Index with dev_e.
const char * dev_txt[] = {
   "DEFAULT",  // preset default device
   "SEARCH1",
   "SEARCH2",
   "SEARCH3",
   "MIDI",
   "TIMIDITY",
   "FLUIDSYNTH",
   "EXT_MIDI",
   "SYNTH"
   "FM_SYNTH",
   "AWE32_SYNTH",
   "DEV6",
   "DEV7",
   "DEV8",
   "DEV9",
   "LIST"
};


// Select preference device.
static
byte  select_device( char ch )
{
    byte sd;
    switch( ch )
    {
     case 'M':
        sd = DVT_MIDI;
        break;
     case 'E':
        sd = DVT_EXT_MIDI;
        break;
     case 'T':
        sd = DVT_TIMIDITY;
        break;
     case 'L':
        sd = DVT_FLUIDSYNTH;
        break;
     case 'S':
        sd = DVT_SYNTH;
        break;
     case 'F':
        sd = DVT_FM_SYNTH;
        break;
     case 'A':
        sd = DVT_AWE32_SYNTH;
        break;
     default:
     case 'a':
        sd = DVT_SEARCH1;
        break;
     case 'b':
        sd = DVT_SEARCH2;
        break;
     case 'c':
        sd = DVT_SEARCH3;
        break;
     case 'd':
        sd = DVT_DEFAULT;
        break;
     case 'g':
        sd = DVT_DEV6;
        break;
     case 'h':
        sd = DVT_DEV7;
        break;
     case 'j':
        sd = DVT_DEV8;
        break;
     case 'k':
        sd = DVT_DEV9;
        break;
    }
    return sd;
}

static
void  parse_option_string( const char * optstr )
{
    const char * p = optstr;

    option_pending = 0;
    if( option_string == NULL )  return;

    while( *p != 0 )
    {
        while(*p == ' ') p++;
        if( *(p++) != '-' )  continue;
       
        switch( *(p++) )
        {
         case 'v':
            verbose=1;
            if( *p )
              verbose= *p - '0';
            break;
         case 'V':
            // fixed volume
            while(*p == ' ') p++;
            vol_change(atoi(p));
            while( isdigit(*p) ) p++;
            changevol_allowed = 0;  // fixed volume
            break;
         case 'd':
            while(*p == ' ') p++;
            if( isalpha( *p ) )
            {
                sel_dvt = select_device( *(p++) );
                while(*p == ' ') p++;
            }
            if( isdigit( *p ) )
            {
                dev_port_num = atoi(p);
                while( isdigit(*p) ) p++;
            }
            break;
         default:
            break;
        }
    }
}

// Return optional parameter value on the command line switch.
// Return -1 when none.
//  avstr : string after the switch
//  avp :  & *argv[]  /*INOUT*/
int  command_value( char * const avstr, /*INOUT*/ char ** avp[] )
{
    if( avstr && *avstr )
    {
      // Check for number at next char of switch.
      if( isdigit( *avstr ) )
         return  atoi( avstr );
    }
   
    if( avp && *avp )
    {
      // Check for number in token following the switch.
      char * next_arg = (*avp)[1];
      if( next_arg )
      {
        if( isdigit( *next_arg ) )
        {
          (*avp)++;  // argument used up
          return  atoi( next_arg );
        }
      }
    }
    return -1;  // invalid
}


// Parse the command line.
void  command_line( int ac, char * av[] )
{
    char * avstr;
    int val;

    if( ac < 1 )  return;

    // Gave up on getopt, as it could not handle our optional arguments.
    av++; // skip program name
    for( ; *av ; av++ )
    {
      avstr = *av;
      if( *avstr == '-' )
      {
        char swch = avstr[1];
        avstr+=2;  // skip - and char
        switch ( swch )
        {
         case 'd':
            if( *avstr == 0 )
            {
                if( *av[1] == '-' )  continue;
                // Select letter must be in next token.
                av++;
                avstr = *av;
            }
            if( isalpha( *avstr ) )
                sel_dvt = select_device( *(avstr++) );
            dev_port_num = command_value( avstr, &av );  // optional port num
            break;
         case 'u':
            dev_type = command_value( avstr, &av );
            break;
         case 'l':
            list_devs();
            break;
         case 't':
            val = command_value( avstr, &av ); // optional time
            if( val >= 0 )
              timeout = val;  // seconds
            break;
         case 'V':
            changevol_allowed = 0;  // fixed volume
            val = command_value( avstr, &av );  // optional volume
            if( val >= 0 )
            {
              vol_change(val);
            }
            break;
         case 'c':
            parent_check = 0;
            break;
         case 'v':
            verbose = 1;
            val = command_value( avstr, &av );  // optional verbose level
            if( val > 0 )
              verbose = val;
            break;
         case 'h':
            show_help();
            exit(0);
            break;
         case 'x':
            no_devices_exit = 1;
            break;
         case '?': case ':':
            show_help();
            exit(1);
            break;
        }
      }
    }
   
//    printf( "dev_sel= %s  dev_port= %i  dev_type= %i\n", dev_txt[sel_dvt], dev_port_num, dev_type );

#ifndef AWE32_SYNTH_SUPPORT
    if( sel_dvt == DVT_AWE32_SYNTH )
    {
        printf("musserver: No AWE32 support\n");
        sel_dvt = DVT_DEFAULT;
    }
#endif
}



// Cleanup and Exit
void cleanup_exit(int status, char * exit_msg)
{
    struct msqid_ds *dummy;
    
    cleanup_midi();
    dummy = malloc(sizeof(struct msqid_ds));
    msgctl(qid, IPC_RMID, dummy);
    free(dummy);
    if( infile )
      fclose(infile);

    if( (status > 1) || (status < -1) || verbose )
    {
      if( exit_msg )
      {
        if( strlen(exit_msg) > 0 )
          printf( "musserver: %s.\n", exit_msg );
        printf( "musserver: exiting.\n" );
      }
    }
    exit(status);
}


int main(int argc, char **argv)
{
    char * fail_msg = "";
    unsigned int musicsize;
    pid_t ppid;
    int timeout_cnt;

    music_data.data = NULL;
    music_data.size = 0;

#if defined(SCOOS5)
    parent_check = 0;
#endif
    command_line( argc, argv );
  
    if( verbose )
        printf("musserver version %s\n", MUS_VERSION);

    ppid = getpid();  // our pid
    if(verbose >= 2) 
        printf("musserver pid %d\n", ppid);

    if( parent_check )
    {
        ppid = getppid();  // parent pid
        sprintf(parent_proc, "/proc/%d", (int)ppid);  // length 17
        if (verbose >= 2)
            printf("parent pid %d %s\n", ppid, parent_proc);
        if( ppid < 2 )
            parent_check = 0;  // started by init, such as from system() call.
        // Will get correct PPID sent by IPC.
    }

    // The message queue is created after the musserver is started.
    qid = -9;
    for( timeout_cnt = timeout * 1000 / TIMEOUT_UNIT_MS; ; timeout_cnt-- )
    {
        // Even if timeout is 0, test for IPC queue at least once.
        qid = msgget( MUSSERVER_MSG_KEY, 0);
        // Cannot have a printf before checking errno !
        if (qid >= 0 )  break;

        switch(errno)
        {
          case ENOENT:  // does not exist yet
            if ((verbose >= 2) && ((timeout_cnt & 0x0F) == 4))
            {
               printf("Waiting for message queue id...\n");
            }
            break;
          case EACCES:  // do not have access permission
            fail_msg="IPC message queue, permission failure";
            goto  fail_exit;
          case ENOMEM:  // do not enough memory
            fail_msg="IPC message queue, not enough memory";
            goto  fail_exit;
          case ENOSPC:  // system limit
            fail_msg="IPC message queue, system limit";
            goto  fail_exit;
          default:
            fail_msg="IPC message queue, general failure";
            goto  fail_exit;
        }
        if( timeout_cnt < 1 )
        {
            fail_msg="Could not connect to IPC";
            goto  fail_exit;
        }
        usleep(TIMEOUT_UNIT_MS * 1000);  // 0.2 sec
    }
    if (verbose >= 2)
        printf("qid: %d\n", qid);

    if (verbose >= 2)
        printf("Waiting for first message from Doom...\n");

    // The DoomLegacy wad search is very complicated, and game dependent.
    // PWAD may also be involved.
    // Get the wad file name from IPC.

    while(genmidi_lump.state == PLAY_OFF)
       get_mesg(MSG_WAIT);

    if( genmidi_lump.wad_name == NULL )
      goto normal_exit_terminate;
   
    if( option_pending )
    {
        // Parse the option string from doom
        parse_option_string( option_string );
    }
   
    if( verbose >= 2 )
       printf( " select sel_dvt=%s, dev_type=%i, port=%i\n", dev_txt[sel_dvt], dev_type, dev_port_num );
    // init, load, setup the selected device
    seq_midi_init_setup(sel_dvt, dev_type, dev_port_num);
   
    // Instrument setup is done.
    
    // Wait for first music
    while(music_lump.state == PLAY_OFF)
       get_mesg(MSG_WAIT);
   
    if( music_lump.wad_name == NULL )
      goto normal_exit_terminate;

    for(;;)
    {
        if( option_pending )
        {
            parse_option_string( option_string );
            cleanup_midi();
            // load, setup the selected device
            seq_midi_init_setup(sel_dvt, dev_type, dev_port_num);
        }
       
        if( music_lump.state != PLAY_START )
        {
            get_mesg(MSG_WAIT);
        }

        if( music_lump.state != PLAY_START )  continue;

        if (verbose >= 2)
            printf("Playing music resource number %d\n", music_lump.lumpnum + 1);
        musicsize = read_wad_music( & music_lump,
             /* OUT */  & music_data );
        if( musicsize )
        {
            // Looping is now set by IPC message.
            playmus( & music_data, 1 );
        }
        switch ( music_lump.state )
        {
          case PLAY_START:
            break;
          case PLAY_STOP:
            free(music_data.data);
            music_data.data = NULL;
            break;
          case PLAY_RESTART:
            music_lump.state = PLAY_START;
            break;
          case PLAY_QUITMUS:
            if (verbose)
                printf("Terminated\n");
            goto normal_exit_terminate;
          default:
            fail_msg = "unknown error in music playing";
            goto fail_exit;
        }
    }

normal_exit_terminate:
    cleanup_exit(0, NULL);
    return 0;

fail_exit:
    cleanup_exit(2, fail_msg);
    return 1;
}
