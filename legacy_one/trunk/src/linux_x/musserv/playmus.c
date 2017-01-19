// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1995-1996 Michael Heasley (mheasley@hmc.edu)
//   GNU General Public License
// Portions Copyright (C) 1996-2016 by DooM Legacy Team.
//   GNU General Public License
//   Heavily modified for use with Doom Legacy.
//   Removed wad search and Doom version dependencies.
//   Is now dependent upon IPC msgs from the Doom program
//   for all wad information, and the music lump id.

/*************************************************************************
 *  playmus.c
 *
 *  Copyright (C) 1995-1996 Michael Heasley (mheasley@hmc.edu)
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/soundcard.h>
#include <unistd.h>
#ifdef linux
#  include <signal.h>
#  include <errno.h>
#elif defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7) || defined(__FreeBSD__)
#  include <sys/signal.h>
#  include <errno.h>
#endif
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "musserver.h"


music_wad_t  music_lump = { NULL, 0 };
music_wad_t  genmidi_lump = { NULL, 0 };

byte  continuous_looping = 0;
byte  music_paused = 0;
byte  option_pending = 0;  // msg has set the option string

char * option_string = NULL;

// Cannot limit music to doom1 and doom2 names, must play any music lump.

//-- Signal functions

void sigfn_quitmus()
{
  if( music_lump.wad_name )
      free( music_lump.wad_name );
  music_lump.wad_name = NULL;
  music_lump.lumpnum = TERMINATED;
  music_lump.state = PLAY_QUITMUS;
}

void sigfn_do_nothing()
{
  signal(SIGHUP, sigfn_do_nothing);
}

//-- Service the IPC messages

// In milliseconds
#define MSG_CHECK_MS  400


void get_mesg(byte wait_flag)
{
  // [WDJ] This is called often, so do not malloc the buffer.
  static mus_msg_t  recv;  // size MUS_MSG_MTEXT_LENGTH
  static uint32_t   parent_check_counter = 0;
  static char *  wadname = NULL; // malloc

  int result;
  char * fail_msg = "";
  unsigned int msgflags = MSG_NOERROR;
  int repeat = 1;

  if( ! wait_flag )
     msgflags |= IPC_NOWAIT;

  while (repeat)
  {
    repeat = 0;
    result = msgrcv(qid, &recv, MUS_MSG_MTEXT_LENGTH, 0, msgflags );
    if (result > 0)
    {
      if (verbose >= 2)
        printf("ipc: bytes = %d, mtext = %s\n", result, recv.mtext);
       
      switch (recv.mtext[0])
      {
       case 'v': case 'V':
        if (changevol_allowed)
        {
          int vol = atoi(&recv.mtext[2]);
          vol_change( vol );
          if (verbose >= 2)
            printf("musserver: volume change = %d\n", vol);
        }
        repeat = 1;
        break;
       case 'o': case 'O':  // Options
        if( option_string )
          free( option_string );
        option_string = strdup( &recv.mtext[1] );
        option_pending = 1;
        if( music_lump.state == PLAY_RUNNING )
          music_lump.state = PLAY_RESTART;
        break;
       case 'd': case 'D':
        if (verbose)
          printf("music name = %s\n", &recv.mtext[1]);
        // no longer search for music names, is only informational
        break;
       case 'w': case 'W':
        // Wad name
        if( wadname )
          free( wadname );
        wadname = strdup( &recv.mtext[1] );  // malloc
        repeat = 1;  // Another msg expected.
        break;
       case 'g': case 'G':
        // GenMidi lump number.
        if( genmidi_lump.wad_name )
            free( genmidi_lump.wad_name );
        genmidi_lump.wad_name = wadname;
        genmidi_lump.lumpnum = atoi(&recv.mtext[2]);
        genmidi_lump.state = PLAY_START;
        if (verbose >= 2)
          printf("genmidi lump number = %d, in %s\n", genmidi_lump.lumpnum,
                 (wadname? wadname:"WAD NAME MISSING") );
        wadname = NULL;
        break;
       case 's': case 'S':
        // Music lump number.
        if( music_lump.wad_name )
            free( music_lump.wad_name );
        music_lump.wad_name = wadname;
        music_lump.lumpnum = atoi(&recv.mtext[2]);
        music_lump.state = PLAY_START;
        continuous_looping = (toupper(recv.mtext[1]) == 'C');
        if (verbose >= 2)
        {
          printf("musserver: Start %s, music lump number = %d, in %s\n",
                 (continuous_looping?"Cont":""), music_lump.lumpnum,
                 (wadname?wadname:"WAD NAME MISSING") );
        }
        wadname = NULL;
        break;
       case 'p': case 'P':
        // Pause and Stop
        music_paused = atoi(&recv.mtext[2]);
        if( music_paused )
        {
            if(music_lump.state == PLAY_RUNNING )
              music_lump.state = PLAY_PAUSE;
        }
        else
        {
            if(music_lump.state == PLAY_PAUSE )
              music_lump.state = PLAY_RUNNING;
        }
        break;
       case 'x': case 'X':
        // Stop song
        music_lump.state = PLAY_STOP;
        break;
       case 'i': case 'I':
        {
          // Watch PPID
          int ppid = atoi(&recv.mtext[2]);
          sprintf(parent_proc, "/proc/%d", ppid);  // length 17
          if( ppid > 1 )
            parent_check = 1;
        }
        break;
       case 'q': case 'Q':
        // Quit
        music_lump.state = PLAY_QUITMUS;
        if( verbose )
          printf("musserver: Received QUIT\n");
        // Close the queue.
        msgctl(qid, IPC_RMID, (struct msqid_ds *) NULL);
        return;
      }
    }
    else if (result < 0)
    {
      switch (errno)
      {
       case EACCES:  // do not have access permission
        fail_msg="IPC message queue, permission failure";
        goto  fail_exit;
       case ENOMEM:  // do not enough memory
        fail_msg="IPC message queue, not enough memory";
        goto  fail_exit;
       case ENOSPC:  // system limit
        fail_msg="IPC message queue, system limit";
        goto  fail_exit;
       case EFAULT:
        fail_msg="IPC message queue, memory address is inaccessible";
        goto  fail_exit;
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
       case EIDRM:
        // Queue was removed while process was waiting for a message.
        // Can only get EIDRM is process is sleeping while waiting for message.
        fail_msg="IPC message queue, message queue has been removed";
        goto  fail_exit;
#endif
       case EINTR:
        if (verbose)
          printf("IPC message queue: received an interrupt signal\n");
        break;
       case EINVAL:
        // Queue does not exist, or other errors.
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
        fail_msg="IPC message queue, invalid message size or queue id";
#elif defined(__FreeBSD__)
        fail_msg="IPC message queue, message queue has been removed or invalid queue id";
#else
        fail_msg="IPC message queue, invalid message queue id";
#endif
        goto  fail_exit;
#ifdef __FreeBSD__
       case E2BIG:
        fail_msg="IPC message queue, invalid message size or queue id";
        goto  fail_exit;
#endif
#if defined(linux) || defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
       case ENOMSG:
        // The normal case when there is no message.
        break;
#elif defined(__FreeBSD__)
       case EAGAIN:
        break;
#endif
       default:
//	 fail_msg="IPC message queue, general failure";
         fail_msg=strerror(errno);
         goto  fail_exit;
      }
    }
  }

  // Check on the parent, every few messages.
  if (parent_check && ((parent_check_counter++ & 0x07) == 0x06))
  {
    FILE *tmp;
    /* Check to see if doom is still alive... */
    if ((tmp = fopen(parent_proc, "r")) != NULL)
      fclose(tmp);
    else
    {
      cleanup_exit(1, "parent process appears to be dead");
    }
  }
  return;
  
fail_exit:
  cleanup_exit(2, fail_msg);
  return;
}


//#define THROTTLE_MIDI

#ifdef THROTTLE_MIDI
// get_float_time is relative to this value, to preserve significant digits
time_t start_sec = 0;

void  start_float_time(void)
{
    struct timeval      tp;

    gettimeofday(&tp, NULL);  // seconds since EPOCH
    start_sec = tp.tv_sec;
}

// returns time in seconds
double  get_float_time(void)
{
    struct timeval      tp;

    gettimeofday(&tp, NULL);  // seconds since EPOCH

    return (double)(tp.tv_sec - start_sec) + (((double)tp.tv_usec)/1000000.0);
}

#endif


void playmus(music_data_t * music_data, byte check_msg)
{
  byte * musp;
  byte event0, event1, event2;  // bytes of the event
  byte eventtype;
  byte channelnum;
  byte lastflag = 0;
  byte notenum;
  byte notevol;
  unsigned int pitchwheel;
  byte controlnum;
  byte controlval;
  unsigned int ticks;
  byte tdata;
  unsigned int lastvol[16];
  int queue_thresh;
  double delaytime;
  double curtime = 0.0;
#ifdef THROTTLE_MIDI
  double clktime, target_time;
#endif

  signal(SIGHUP, sigfn_do_nothing);
  signal(SIGQUIT, sigfn_quitmus);
  signal(SIGINT, sigfn_quitmus);
  signal(SIGTERM, sigfn_quitmus);
  signal(SIGCONT, SIG_IGN);

  // To keep from filling the sequencer queue (which hurts responsiveness).
  queue_thresh = queue_size - 64;

  musp = music_data->data;

  reset_midi();
  midi_timer(MMT_START);

  music_lump.state = PLAY_RUNNING;

#ifdef THROTTLE_MIDI
  start_float_time();
#endif
  
  for(;;)
  {
    // Midi event, variable length.
    // 7 bit data, sign bit is a flag.
    event0 = *musp++;
    channelnum = event0 & 0x0F;
    if (channelnum > 8)
      channelnum++;
    if (channelnum == 16)
      channelnum = 9;  // drum
    eventtype = (event0 >> 4) & 0x07;
    lastflag = event0 & 0x80;

    switch (eventtype)
    {
    case 0:		/* note off */
      event1 = *musp++;
      notenum = event1 & 0x7F;
      note_off(notenum, channelnum, lastvol[channelnum]);
      break;

    case 1:		/* note on */
      event1 = *musp++;
      notenum = event1 & 0x7F;
      if (event1 & 0x80)
      {
        event2 = *musp++;
        notevol = event2 & 0x7F;
        lastvol[channelnum] = notevol;
      }
      note_on(notenum, channelnum, lastvol[channelnum]);
      break;

    case 2:		/* pitch wheel */
      event1 = *musp++;
      pitchwheel = event1 / 2;
      pitch_bend(channelnum, pitchwheel);
      break;

    case 4:		/* midi controller change */
      event1 = *musp++;
      controlnum = event1 & 0x7F;
      event2 = *musp++;
      controlval = event2 & 0x7F;
      switch (controlnum)
      {
        case 0:		/* patch change */
          patch_change(controlval, channelnum);
          break;
        case 3:		/* volume */
          control_change(CTL_MAIN_VOLUME, channelnum, controlval);
          break;
        case 4:		/* pan */
          control_change(CTL_PAN, channelnum, controlval);
          break;
      }
      break;

     case 6:	/* end of music data */
        if ( continuous_looping )
        {
          all_off_midi();
          usleep(100000);
          // restart music
          musp = music_data->data;
          midi_timer(MMT_START);
          curtime = 0.0;
          lastflag = 0;
        }
        else
        {
          // play once
          // Wait until message changes state.
          get_mesg(MSG_WAIT);
        }
      break;

    case 3:	/* unknown, but contains data */
      musp++;
      break;

    default:	/* unknown */
      break;
    }

    if (lastflag)	/* next data portion is time data */
    {
      // get delay time, multiple bytes, most sig first
      tdata = *musp++;
      ticks = tdata & 0x7F;
      while(tdata & 0x80)
      {
        tdata = *musp++;
        ticks = (ticks * 128) + (tdata & 0x7F);
      }
      // Wait for delaytime,  while checking for messages.
      // Observed delay 0.7 .. 28.0

      // delay is in 128ths of quarter note, which depends upon tempo.
      // approx.  100th second
      delaytime = (double)ticks / 1.4;
      curtime += delaytime;
      // midi_wait syncs the queue, which may sleep,
      // but it is independent of delaytime.
      midi_wait( (uint32_t) curtime);

      // Midi wait only sends a midi event, and returns immediately;
      // it does NOT stall the caller.
      // This means that the midi events sent will be queued up far ahead
      // of the note playing.
      // If we stop or pause, it will take awhile (the queue size)
      // for the music to actually stop.
      
#ifdef THROTTLE_MIDI
      // Do not let curtime get too far ahead of clktime.
      clktime = get_float_time();
      target_time = (curtime/100) - (4*MSG_CHECK_MS/1000.0);
//      printf( "clktime=%f, target=%f\n", clktime, target_time );
      for( ; ; )
      {
        // constant time increments between message checks
//      printf( "  floattime=%f\n", get_float_time() );
        if( get_float_time() > target_time )  break;
        if (check_msg)
        {
          get_mesg(MSG_NOWAIT);
          if (music_lump.state != PLAY_RUNNING)  goto handle_msg;
        }
        usleep( MSG_CHECK_MS * 1000 );
      }
#endif       
    }

    if (check_msg)
    {
      get_mesg(MSG_NOWAIT);
      if (music_lump.state != PLAY_RUNNING)  goto handle_msg;
    }
    // To limit amount of music already in the queue.
    while( queue_free() < queue_thresh )
    {
      usleep( MSG_CHECK_MS * 1000 );
    }
    continue;

handle_msg:
    if( music_lump.state == PLAY_PAUSE )
    {
      if( verbose >= 2 )
        printf( "Music paused\n" );
      pause_midi();
      while( music_lump.state == PLAY_PAUSE )
        get_mesg( MSG_WAIT );
      if (music_lump.state != PLAY_RUNNING)  break;
      // There is no unpause, unless want to restore all note states.
    }
    if( music_lump.state != PLAY_RUNNING )  break;
    if( verbose >= 2 )
      printf( "Music run\n" );
  } // play loop

  if( verbose >= 2 )
    printf( "Music STOP\n" );
  reset_midi();
  return;
}
