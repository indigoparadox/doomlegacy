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
 *  musserver.h
 *
 *  Copyright (C) 1995 Michael Heasley (mheasley@hmc.edu)
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

#ifndef MUSSERVER_H


/**************************************************/
/* User-configurable parameters: program defaults */
/**************************************************/


/*************************************************************************
 * Change this to your preferred default playback device: external midi, *
 * FM synth, or AWE32 synth                                              *
 *************************************************************************/

/* #define DEFAULT_EXT_MIDI */
#define DEFAULT_FM_SYNTH
/* #define DEFAULT_AWE32_SYNTH */


/************************************************************************
 * To compile in support for AWE32 synth (requires AWE32 kernel driver, *
 * see README) regardless of the default playback device, define the    *
 * following                                                            *
 ************************************************************************/

/* #define AWE32_SYNTH_SUPPORT */


/***************************************************************************
 * If you normally need the -u command-line switch to specify a particular *
 * device type, uncomment this line and change the type as needed          *
 ***************************************************************************/

/* #define DEFAULT_TYPE 8 */


// A unique key for getting the right queue.
#define MUSSERVER_MSG_KEY  ((key_t)53075)

/************************************/
/* End of user-configurable section */
/************************************/


#ifdef DEFAULT_AWE32_SYNTH
#  define AWE32_SYNTH_SUPPORT
#endif

#ifdef linux
#  include <sys/soundcard.h>
#  ifdef AWE32_SYNTH_SUPPORT
#    include <linux/awe_voice.h>
#  endif
#elif defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#  include <sys/soundcard.h>
#  ifdef AWE32_SYNTH_SUPPORT
#    include <sys/awe_voice.h>
#  endif
#elif defined(__FreeBSD__)
#  include <machine/soundcard.h>
#  ifdef AWE32_SYNTH_SUPPORT
#    include <awe_voice.h>
#  endif
#endif

#define MUS_VERSION "1.4_DoomLegacy"

#include <stdint.h>
#include "doomtype.h"
  // byte
//typedef unsigned char   byte;
#include "doomdef.h"
  // MAX_WADPATH

// Length of mtext, which now includes a directory/filename.
#define MUS_MSG_MTEXT_LENGTH    MAX_WADPATH
// This message structure is dictated by message operations msgrcv, msgsnd.
typedef struct
{
    long  mtype;      /* type of received/sent message */
    char  mtext[MUS_MSG_MTEXT_LENGTH];  /* text of the message */
} mus_msg_t;

#if __GNUC_PREREQ(2,2)
  // Since glibc 2.2 this parameter has been void* as required by SUSv2 an SUSv3.
# define MSGBUF(x)  ((void*)&(x))
#else
  // Previous to glib 2.2
# define MSGBUF(x)  ((struct msgbuf*)&(x))
#endif



typedef struct {
    uint16_t   flags;
    byte   finetune;
    byte   note;
    sbi_instr_data   patchdata;
} opl_instr_t;

typedef struct {
    int32_t    note;
    int32_t    channel;
} synth_voice_t;


// Device types
typedef enum {
   DVT_DEFAULT, // preset default device
   DVT_SEARCH1,
   DVT_SEARCH2,
   DVT_SEARCH3,
   DVT_MIDI,  // TIMIDITY, FLUIDSYNTH, then EXT_MIDI
   DVT_TIMIDITY,
   DVT_FLUIDSYNTH,
   DVT_EXT_MIDI,
   DVT_SYNTH,  // synth devices
   DVT_FM_SYNTH,
   DVT_AWE32_SYNTH,
   DVT_DEV6,
   DVT_DEV7,
   DVT_DEV8,
   DVT_DEV9,
   DVT_LIST,
} dvtype_e;

// music play state, set by IPC messages
typedef enum {
   PLAY_OFF,
   PLAY_START,
   PLAY_RUNNING,
   PLAY_PAUSE,
   PLAY_STOP,
   PLAY_RESTART,
   PLAY_QUITMUS
} play_e;

#define TERMINATED 0xFFFFF

typedef struct {
  char * wad_name;  // malloc
  int  lumpnum;
  byte state;   // play_e
} music_wad_t;

typedef struct {
  uint32_t  filepos;  // position in file
  uint32_t  size;  // music data size
  byte *  data;  // music data, malloc
} music_data_t;

// from IPC message
extern char * option_string;

extern music_wad_t  music_lump;
extern music_wad_t  genmidi_lump;

extern byte option_pending;  // msg has set the option string
extern byte continuous_looping;
extern byte music_paused;
extern byte verbose;
extern byte changevol_allowed;
extern byte parent_check;  // check parent process
extern byte no_devices_exit;
extern char parent_proc[32];  // parent process /proc/num

extern int queue_size;
int queue_free( void );


// Music midi timer.
typedef enum { MMT_START, MMT_STOP, MMT_CONT }  mmt_e;
// action : mmt_e
void midi_timer(int action);

void all_off_midi(void);
void pause_midi(void);
void reset_midi(void);
void midi_wait(uint32_t wtime);

void seqbuf_dump(void);
void note_off(int note, int channel, int volume);
void note_on(int note, int channel, int volume);
void pitch_bend(int channel, int value);
void control_change(int controller, int channel, int value);
void patch_change(int patch, int channel);
void vol_change(int volume);

void playmus(music_data_t * music_data, byte check_msg);

extern int qid;  // IPC message queue id

enum{ MSG_NOWAIT=0, MSG_WAIT=1 };
//  wait_flag : MSG_WAIT, MSG_NOWAIT
void get_mesg(byte wait_flag);

// Init, load, setup the selected device
void seq_midi_init_setup(int sel_dev, int dev_type, int port_num);
void cleanup_midi(void);
void list_devs(void);

// Wad read
// Read the GENMIDI lump from a wad.
//  gen_wad : the wad name and lumpnum
void read_wad_genmidi( music_wad_t * gen_wad );

// Return music size.
//  music_wad : the wad name and lumpnum
//  music_data : the music lump read
int read_wad_music( music_wad_t * music_wad,
             /* OUT */  music_data_t * music_data );

// Exit the program.
void cleanup_exit(int status, char * exit_msg);
#endif
