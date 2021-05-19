// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1995-1997 Michael Heasley (mheasley@hmc.edu)
//   GNU General Public License Version 2
// Portions Copyright (C) 1996-2016 by DooM Legacy Team.
//   GNU General Public License Version 2
//   Heavily modified for use with Doom Legacy.
// Date: 2021-3
//   Interface functions modified for DoomLegacy use.

#include "../lx_ctrl.h"
  // MUS_DEVICE_OPTION
  // DEV_xx


// #define DEBUG_MUSIC_INIT

// #define DEBUG_MIDI_CONTROL


// Some synth, like TiMidity, do not implement all-off effectively.
// Take drastic measures to kill notes that drone on.
//#define ALL_OFF_FIX 1

#ifdef DEV_TIMIDITY
// [WDJ] To make TiMidity dump midi commands to console 1,
// add switch "--verbose=3" to where it is started.
// Look in /etc/rc.d/rc.local.
#endif


// Search orders for pref device option.
// Now that this is changable from the DoomLegacy menu,
// no longer do search when an specific device is specified.
// Ext midi is only a port, even when nothing is there, so put it last.
static char * search_order[] =
{
  "",    // DVT_DEFAULT, never used
  "ALTFE",      // DVT_SEARCH1
  "AFLTghjkE",  // DVT_SEARCH2, to be customized
  "kjhgTLFAE",  // DVT_SEARCH3, to be customized
  "TLE",  // DVT_MIDI
  "T",    // DVT_TIMIDITY
  "L",    // DVT_FLUIDSYNTH
  "E",    // DVT_EXT_MIDI
  "AFL",  // DVT_SYNTH
  "F",    // DVT_FM_SYNTH
  "A",    // DVT_AWE32_SYNTH
  "g",    // DVT_DEV6
  "h",    // DVT_DEV7
  "j",    // DVT_DEV8
  "k"     // DVT_DEV9
};



/*************************************************************************
 *  sequencer.c
 *
 *  Copyright (C) 1995-1997 Michael Heasley (mheasley@hmc.edu)
 *
 *  Portions Copyright (c) 1997 Takashi Iwai (iwai@dragon.mm.t.u-tokyo.ac.jp)
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
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/soundcard.h>
  // synth_info
#include "musserver.h"




// Arbitrary dev type value.  The Midi dev types all seem to be 0.
#define DVT_TYPE_TIMIDITY    1001
#define DVT_TYPE_FLUIDSYNTH  1002

//#define SEQ_MIDIOUT(a,b) test(a,b)

#define MAX_SYNTH_INFO   8
#define MAX_MIDI_INFO    8

static struct synth_info  sinfo[MAX_SYNTH_INFO];
// known fields:
//   char name[]
//   int device : 0..n, index to devices
//   int synth_type
//      SYNTH_TYPE_FM, SYNTH_TYPE_SAMPLE, SYNTH_TYPE_MIDI
//   int synth_subtype
//      FM_TYPE_ADLIB
//      FM_TYPE_OPL3
//      MIDI_TYPE_MPU401
//      SAMPLE_TYPE_BASIC
//      SAMPLE_TYPE_GUS
//      SAMPLE_TYPE_WAVEFRONT
//   int nr_voices
//   int instr_bank_size
//   uint capabilities : bit flags
//      SYNTH_CAP_PERCMODE, SYNTH_CAP_OPL3, SYNTH_CAP_INPUT

static struct midi_info   minfo[MAX_MIDI_INFO];
// known fields:
//   char name[]
//   int device : 0..n, index to devices
//   uint capabilities   (almost always 0, except for MPU401)
//      MIDI_CAP_MPU401
//   int dev_type        (always 0)

// Selected device
static struct synth_info *  synth_ip = NULL;
static struct midi_info *   midi_ip = NULL;

static byte use_dvt = DVT_DEFAULT;   // dvt_e
static int seq_dev = -1;  // sound port, and index into sinfo or minfo

// Found devices, port nums.
static int awe_dev = -1;
static int fm_dev = -1;
static int timidity_dev = -1;
static int fluidsynth_dev = -1;
static int ext_midi_dev = -1;

static int seqfd;
static int mixfd;
static int synth_patches[16];
static int chanvol[16];
static int volscale = 100;
static byte fm_note12 = 0;
static synth_voice_t *voices;
static struct sbi_instrument fm_sbi[175];
opl_instr_t  fm_instruments[175];


#if 0
// Indexed by synth_type
static const char * synth_type_txt[] = {
   "FM", //  SYNTH_TYPE_FM
   "SAMPLE", // SYNTH_TYPE_SAMPLE
   "MIDI",  // SYNTH_TYPE_MIDI
};
#endif

// Some operations from soundcard.h cause messages about violating strict aliasing.
// This is due to an operation  *(short *)&_seqbuf[_seqbufptr+6] = (w14).
// There is no good way to stop this, as long as -wstrict-aliasing is set.

// OSS sequencer interface.
SEQ_USE_EXTBUF();
SEQ_DEFINEBUF(2048);

int queue_size = 0;

// This function must be provided by the program, for the sequencer interface.
// This function is as specified in /usr/include/linux/soundcard.h
// to be used by the SEQ_ macros.
// This will sleep() if the sequencer buffer is full.
void seqbuf_dump(void)
{
  // Write the output buffer to the sequencer port.
  if (_seqbufptr)
  {
    if (write(seqfd, _seqbuf, _seqbufptr) == -1)
    {
       perror("write /dev/sequencer");
       cleanup_exit(-1, "Seqbuf dump");
       // NO RETURN
    }
  }
  _seqbufptr = 0;
}


int queue_free( void )
{
  int queue_free;
  // Update the queue status.
  ioctl(seqfd, SNDCTL_SEQ_GETOUTCOUNT, &queue_free );
  return queue_free;
}


// ============= OSS MIDI SECTION


// [WDJ] Notes on ioctrl:
// SNDCTL_SEQ_TESTMIDI: tests midi device for life.
//   Returns EFAULT when user not allowed.
//   Returns ENXIO when midi_dev is invalid.
//   Returns open err, when fails to be opened.
//   Returns 0 when passes. 
// SNDCTL_SEQ_SYNC: attempts to sync queue
//   Returns EINTR if fails to empty queue.
//   This only flushes the music to the device or TiMidity,
//   which has its own queue.
//   Often hangs the musserver.
// SNDCTL_SEQ_GETOUTCOUNT:
//   Put (MAX_QUEUE - qlen) to value.
// SNDCTL_SEQ_RESET:  reset the sequencer
//   Midi: sends all-notes-off to all channels of each midi that was written.
// SEQ_WAIT: Starts a sound timer, returns 1.  Does not sleep or wait.
// SEQ_MIDIOUT: uses command MIDIPUTC to put bytes to a buffer.
//   seqbuf_dump flushes the buffer to the sequencer device.
//   During seqbuf_dump, if there is not enough space in queue, the
//   write will sleep.
// SEQ_MIDIPUTC: Will start sound timer when buffer is full, and returns 2.
//   This is not the device queue.

//  dvt_type : dvt_e
// Return index to midi_info, -1 when fail.
int find_midi(int dvt_type, int dev_type, int port_num)
{
  int num_midi;
  int m = 0;
  int ior;
  
  // Get the number of midi devices.
  if (ioctl(seqfd, SNDCTL_SEQ_NRMIDIS, &num_midi) != 0)
    return 0;

  if( num_midi > MAX_MIDI_INFO )
    num_midi = MAX_MIDI_INFO;

  if( dvt_type != DVT_LIST && port_num >= 0 && port_num < num_midi )
  {
    // Get a specific port.
    minfo[port_num].device = port_num;
    ior = ioctl(seqfd, SNDCTL_MIDI_INFO, &minfo[port_num]);
    if( ior == 0 )
       return port_num;
  }
  
  for (m = 0; m < num_midi; m++)
  {
    // Set device, then ioctl fills in name, capabilities, dev_type.
    minfo[m].device = m;
    ior = ioctl(seqfd, SNDCTL_MIDI_INFO, &minfo[m]);
    if( ior < 0 )  continue;
    if( strstr( minfo[m].name, "TiMidi" ) )
    {
      minfo[m].dev_type = DVT_TYPE_TIMIDITY;
      if( timidity_dev < 0 )
        timidity_dev = m;
    }
    else if( strstr( minfo[m].name, "FLUID" ) )
    {
      minfo[m].dev_type = DVT_TYPE_FLUIDSYNTH;
      if( fluidsynth_dev < 0 )
        fluidsynth_dev = m;
    }
    else
    {
      if( ext_midi_dev < 0 )
        ext_midi_dev = m;
    }
    if( dev_type >= 0 )
    {
      if(minfo[m].dev_type == dev_type)
         return m;
    }
  }
  return num_midi;
}

//  dvt_type : dvt_e
// Return index to synth_info, -1 when fail.
int find_synth(int dvt_type, int dev_type, int port_num)
{
  int num_synth = 0;
  int ior;
  int s = 0;

  if (ioctl(seqfd, SNDCTL_SEQ_NRSYNTHS, &num_synth) == -1)
    return -1;

  if( num_synth > MAX_SYNTH_INFO )
    num_synth = MAX_SYNTH_INFO;

  if( dvt_type != DVT_LIST && port_num >= 0 && port_num < num_synth)
  {
    sinfo[port_num].device = port_num;
    ior = ioctl(seqfd, SNDCTL_SYNTH_INFO, &sinfo[port_num]);
    if( ior == 0 )
       return port_num;
  }
   
  for (s = 0; s < num_synth; s++)
  {
      sinfo[s].device = s;
      ior = ioctl(seqfd, SNDCTL_SYNTH_INFO, &sinfo[s]);
      if( ior < 0 )  continue;
      if (sinfo[s].synth_type == SYNTH_TYPE_SAMPLE)
      {
#ifdef AWE32_SYNTH_SUPPORT
        if (sinfo[s].synth_subtype == SAMPLE_TYPE_AWE32)
        {
          if( awe_dev < 0 )
            awe_dev = s;
          if ((dev_type == SYNTH_TYPE_SAMPLE) && (dvt_type == DVT_AWE32_SYNTH))
            break;
        }
#endif
      }
      else
      {
        if( fm_dev < 0 )
          fm_dev = s;
        if ((sinfo[s].synth_type == dev_type) && (dvt_type == DVT_FM_SYNTH))
          break;
      }
  }
  return num_synth;
}

#ifdef AWE32_SYNTH_SUPPORT
void setup_awe(int awe_dev)
{
  use_dvt = DVT_AWE32_SYNTH;
  seq_dev = awe_dev;
  synth_ip = & sinfo[awe_dev];

  if (verbose)
    printf("Using synth device number %d (%s)\n", seq_dev+1, synth_ip->name);
}
#endif

void setup_midi(int midi_dev)
{
  use_dvt = DVT_MIDI;  // DVT_EXT_MIDI, DVT_TIMIDITY, DVT_FLUIDSYNTH
  seq_dev = midi_dev;
  midi_ip = & minfo[midi_dev];

  if (verbose)
    printf("Using midi device number %d (%s)\n", seq_dev+1, midi_ip->name);
}

void setup_fm(int fm_dev)
{
  char * fail_msg = NULL;
  FILE *sndstat;
  int num_voices;
  int x;

  use_dvt = DVT_FM_SYNTH;
  seq_dev = fm_dev;
  synth_ip = & sinfo[seq_dev];
  fm_note12 = 0;

  // Linux no longer has /dev/sndstat
  sndstat = fopen("/dev/sndstat", "r");
  if( sndstat )
  {
      char sndver[100];
      char * snddate = NULL;

      fgets(sndver, 100, sndstat);
      fclose(sndstat);

      if( verbose > 1 )
          printf( "musserver: sndver=%s\n", sndver );

      // [WDJ] Cannot fix this code properly because do not have the specific
      // hardware they were detecting, and they did not leave comments.
      // It does not exist on Linux 2.4 or Linux 2.6.
      // Previous code was mostly extraneous.
      snddate = strchr( sndver, '-' );
      if( snddate && ( strncmp( snddate+1, "950728", 6 ) == 0) )
         fm_note12 = 1;
  }

  num_voices = synth_ip->nr_voices;
  voices = malloc( num_voices * sizeof(synth_voice_t));
  for (x = 0; x < num_voices; x++)
  {
    voices[x].note = -1;
    voices[x].channel = -1;
  }
  for (x = 0; x < 16; x++)
    synth_patches[x] = -1;

  mixfd = open("/dev/mixer", O_WRONLY, 0);
  if( mixfd < 0 )
  {
    printf( "musserver: /dev/mixer: %s\n", strerror(errno) );
    fail_msg = "Failed to open mixer";
    goto fail_exit;
  }

  if (verbose)
    printf("Using synth device number %d (%s)\n", seq_dev+1, synth_ip->name);
  return;

fail_exit:
  cleanup_exit(2, fail_msg );
  return;
}

void list_devs( void )
{
  int n, num;

  if ((seqfd = open("/dev/sequencer", O_WRONLY, 0)) < 0)
  {
    perror("open /dev/sequencer");
    exit(1);
  }

  printf("Devices found\n");
  num = find_midi(DVT_LIST, -1, -1);
  for (n = 0; n < num; n++)
  {
    printf("  Port %2i: Midi device of type %d (%s)\n",
        n, minfo[n].dev_type, minfo[n].name);
  }
  num = find_synth(DVT_LIST, -1, -1);
  for (n = 0; n < num; n++)
  {
    printf("  Port %2i: Synth device of type %d (%s)\n",
        n, sinfo[n].synth_type, sinfo[n].name);
  }
  exit(0);
}




static
void seq_setup(int pref_dev, int dev_type, int port_num)
{
  int fnd_dev = -1;
  char * pc;  // pref sequence chars

//  printf( "pref_dev = %i, dev_type = %i, port_num = %i\n", pref_dev, dev_type, port_num );
  if ((seqfd = open("/dev/sequencer", O_WRONLY, 0)) < 0)
  {
    perror("open /dev/sequencer");
    exit(1);
  }

  // Get the queue size;
  ioctl(seqfd, SNDCTL_SEQ_GETOUTCOUNT, &queue_size );
  if( verbose )
    printf( " Sequencer queue size= %i\n", queue_size );
   
  // Midi dev_type is usually always 0.
  if((pref_dev == DVT_TIMIDITY) && (dev_type < 0))
     dev_type = DVT_TYPE_TIMIDITY;
  if((pref_dev == DVT_FLUIDSYNTH) && (dev_type < 0))
     dev_type = DVT_TYPE_FLUIDSYNTH;
  
  if( (pref_dev != DVT_DEFAULT)
      && ((dev_type >= 0) || (port_num >= 0)) )
  {
    if( pref_dev >= DVT_SYNTH )
    {
      fnd_dev = find_synth( pref_dev, dev_type, port_num);
    }
    else
    {
      fnd_dev = find_midi( pref_dev, dev_type, port_num);
    }
  }

  if( fnd_dev < 0 )
  {
    if (pref_dev == DVT_DEFAULT)
        pref_dev = DEFAULT_DEV;

#ifdef DEFAULT_TYPE
    if (dev_type == -1)
        dev_type = DEFAULT_TYPE;
#endif
    find_midi( DVT_MIDI, dev_type, -1);
    find_synth( DVT_SYNTH, dev_type, -1);
  }
  
  if( verbose )
  {
    printf("Timidity port: %i\n", timidity_dev );
    printf("Ext midi port: %i\n", ext_midi_dev );
    printf("FM port: %i\n", fm_dev );
    printf("AWE32 port: %i\n", awe_dev );
  }

  if ((timidity_dev < 0) && (ext_midi_dev < 0) && (fm_dev < 0) && (awe_dev < 0 ))
    goto no_devices;

  if( pref_dev < DVT_SEARCH1 || pref_dev > DVT_DEV9 )  // table limits
  {
     pref_dev = DVT_SEARCH1;
  }
  pc = search_order[pref_dev];
  use_dvt = DVT_DEFAULT;
  for( ; ; pc++ )
  {
    switch( *pc )
    {
     case 'T': // Timidity dev
      if( timidity_dev >= 0 )
        setup_midi( timidity_dev );
      break;
     case 'L': // Fluidsynth dev
      if( fluidsynth_dev >= 0 )
        setup_midi( fluidsynth_dev );
      break;
     case 'E': // Ext Midi dev
      if( ext_midi_dev >= 0 )
        setup_midi( ext_midi_dev );
      break;
     case 'F': // FM dev
      if( fm_dev >= 0 )
        setup_fm( fm_dev );
      break;
     case 'A': // Awe32 dev
#ifdef AWE32_SYNTH_SUPPORT
      if (awe_dev >= 0)
        setup_awe(awe_dev);
#endif
      break;
     case 'g':  // new device
     case 'h':  // new device
     case 'j':  // new device
     case 'k':  // new device
      break;
     case 0:  // end of list
      goto no_devices;
    }
    if( use_dvt != DVT_DEFAULT )  break;
  }

  reset_midi();
  return;

no_devices:
  seq_dev = -1;
  if( no_devices_exit )
      cleanup_exit(1, "no music devices found" );
  return;
}




// ---- MIDI

// [WDJ]
// SEQ_MIDIOUT: Puts the 4 byte midi command in the buffer.
// SEQ_START_NOTE: sends MIDI_NOTEON using 8 byte command
// SEQ_STOP_NOTE: sends MIDI_NOTEOFF using 8 byte command

// Midi Channel Mode Messages
enum {
   CMM_RESET_ALL_CONTROLLERS = 0x79,
   CMM_LOCAL_CONTROL = 0x7A,
   CMM_ALL_NOTES_OFF = 0x7B,
   CMM_OMNI_OFF = 0x7C,
   CMM_OMNI_ON = 0x7D,
   CMM_MONO_ON = 0x7E,  // Poly off
   CMM_POLY_ON = 0x7F   // Mono off
};


void cleanup_midi(void)
{
  reset_midi();
  close(seqfd);
  if (use_dvt == DVT_FM_SYNTH)
    close(mixfd);
}


#ifdef ALL_OFF_FIX
// Some midi synth do not implement all-off, and will leave
// notes running (drones) when the stream stops.
// This tracks what notes and channels were used, and stops them.

// The number of notes and channels actually used in a song is limited.
byte channel_used[16];
byte note_used[0x7F];

static
void clear_used(void)
{
    memset( channel_used, 0, sizeof(channel_used) );
    memset( note_used, 0, sizeof(note_used) );
}
#endif

void all_off_midi(void)
{
  if (use_dvt == DVT_MIDI)
  {
#ifdef ALL_OFF_FIX
    unsigned int note, channel;
    for (channel = 0; channel < 16; channel++)
    {
      if( ! channel_used[channel] ) continue;
      for( note=0; note <= 0x7F; note++ )
      {
        if( note_used[note] )
        {
          SEQ_MIDIOUT(seq_dev, MIDI_NOTEOFF | channel);
          SEQ_MIDIOUT(seq_dev, note);
          SEQ_MIDIOUT(seq_dev, 8);
          SEQ_DUMPBUF();
        }
      }
    }
    clear_used();
#else
    unsigned int channel;
    for (channel = 0; channel < 16; channel++)
    {
      /* all notes off */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE | channel);
      SEQ_MIDIOUT(seq_dev, CMM_ALL_NOTES_OFF);
      SEQ_MIDIOUT(seq_dev, 0);
    }
    SEQ_DUMPBUF();
#endif
  }
}



void pause_midi(void)
{
  // Pause as much as can be paused.
  // Stop the notes.
  unsigned int channel;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    AWE_SET_CHANNEL_MODE(seq_dev, 1);
    AWE_NOTEOFF_ALL(seq_dev);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    for (channel = 0; channel < 16; channel++)
    {
      /* all notes off */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE | channel);
      SEQ_MIDIOUT(seq_dev, CMM_ALL_NOTES_OFF);
      SEQ_MIDIOUT(seq_dev, 0);
    }
  }
  else
  {
    for (channel = 0; channel < sinfo[seq_dev].nr_voices; channel++)
    {
        SEQ_STOP_NOTE(seq_dev, channel, voices[channel].note, 64);
        voices[channel].note = -1;
        voices[channel].channel = -1;
    }
  }
  SEQ_DUMPBUF();
}

void reset_midi(void)
{
  unsigned int channel;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    AWE_SET_CHANNEL_MODE(seq_dev, 1);
    AWE_NOTEOFF_ALL(seq_dev);
    for (channel = 0; channel < 16; channel++)
    {
      SEQ_BENDER_RANGE(seq_dev, channel, 200);
      SEQ_BENDER(seq_dev, channel, 0);
    }
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    // SNDCTL_SEQ_SYNC hangs the musserver
//    ioctl(seqfd, SNDCTL_SEQ_SYNC);
    // All notes off on used channels.
    // Being implemented at the driver, it has the most immediate effect.
    ioctl(seqfd, SNDCTL_SEQ_RESET);
    usleep( 500 );  // long enough for synth to react

#if 0
    // Optional additional all notes off.
    // It does not seem to affect anything much.
    pause_midi();
    // It takes a while for buffer to get to all notes off, and then
    // there is an off decay.
    // If other commands follow too closely, they retrigger the note.
    usleep(20000);
#endif

#ifdef ALL_OFF_FIX
    all_off_midi();  // for synth that drone on
    // Touching the controls too soon seems to retrigger drone
    usleep(20000);
#endif

    for (channel = 0; channel < 16; channel++)
    {
      /* reset pitch bender */
      pitch_bend( channel, 64 );
      /* reset volume to 100 */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE | channel);
      SEQ_MIDIOUT(seq_dev, CTL_MAIN_VOLUME);
      SEQ_MIDIOUT(seq_dev, volscale);
      chanvol[channel] = 100;
      /* reset pan */
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE | channel);
      SEQ_MIDIOUT(seq_dev, CTL_PAN);
      SEQ_MIDIOUT(seq_dev, 64);

      SEQ_DUMPBUF();
    }
  }
  else
  {
    for (channel = 0; channel < sinfo[seq_dev].nr_voices; channel++)
    {
        SEQ_STOP_NOTE(seq_dev, channel, voices[channel].note, 64);
        SEQ_BENDER_RANGE(seq_dev, channel, 200);
        voices[channel].note = -1;
        voices[channel].channel = -1;
    }
  }
  SEQ_DUMPBUF();
  usleep( 300 );  // long enough for synth to react
}

//  channel :  0..15
void note_off(int note, int channel, int volume)
{
  int x = 0;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    SEQ_STOP_NOTE(seq_dev, channel, note, volume);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    SEQ_MIDIOUT(seq_dev, MIDI_NOTEOFF | channel);
    SEQ_MIDIOUT(seq_dev, note);
    SEQ_MIDIOUT(seq_dev, volume);  // velocity
      // some controllers use NOTEOFF velocity, some don't
  }
  else if( synth_ip )
  {
    for (x = 0; x < synth_ip->nr_voices; x++)
    {
      if ((voices[x].note == note) && (voices[x].channel == channel))
      {
        voices[x].note = -1;
        voices[x].channel = -1;
        SEQ_STOP_NOTE(seq_dev, x, note, volume);
        break;
      }
    }
  }
  SEQ_DUMPBUF();
}


//  channel :  0..15
void note_on(int note, int channel, int volume)
{
  int x = 0;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    SEQ_START_NOTE(seq_dev, channel, note, volume);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    SEQ_MIDIOUT(seq_dev, MIDI_NOTEON | channel);
    SEQ_MIDIOUT(seq_dev, note);  // General Midi has assigned codes to notes, 0..127
    SEQ_MIDIOUT(seq_dev, volume);  // velocity  0..127
#ifdef ALL_OFF_FIX
    channel_used[channel] = 1;
    note_used[note] = 1;
#endif
  }
  else if( synth_ip )
  {
    // Find an empty voice
    for (x = 0; x < synth_ip->nr_voices; x++)
    {
      if ((voices[x].note == -1) && (voices[x].channel == -1))
        break;
    }
    if (x < synth_ip->nr_voices)
    {
      voices[x].note = note;
      voices[x].channel = channel;
      if (channel == 9)         /* drum note */
      {
        if (use_dvt == DVT_FM_SYNTH)
        {
          SEQ_SET_PATCH(seq_dev, x, note + 93);
          note = fm_instruments[note + 93].note;
        }
        else
          SEQ_SET_PATCH(seq_dev, x, note + 128);
      }
      else
      {
        SEQ_SET_PATCH(seq_dev, x, synth_patches[channel]);
        if ( fm_note12 )  // [WDJ] have no idea what this fixes
          note = note + 12;
      }
      SEQ_START_NOTE(seq_dev, x, note, volume);
    }
  }
  SEQ_DUMPBUF();
}

void pitch_bend(int channel, signed int value)
{
  int x;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    SEQ_BENDER(seq_dev, channel, 128 * value);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    SEQ_MIDIOUT(seq_dev, MIDI_PITCH_BEND | channel);
    SEQ_MIDIOUT(seq_dev, value >> 7);  // upper 7 bits
    SEQ_MIDIOUT(seq_dev, value & 127); // lower 7 bits
  }
  else if( synth_ip )
  {
    for (x = 0; x < synth_ip->nr_voices; x++)
    {
      if (voices[x].channel == channel)
      {
        SEQ_BENDER_RANGE(seq_dev, x, 200);
        SEQ_BENDER(seq_dev, x, 128*value);
      }
    }
  }
  SEQ_DUMPBUF();
}

void control_change(int controller, int channel, int value)
{
  int x;

  if (controller == CTL_MAIN_VOLUME)
  {
    chanvol[channel] = value;
    value = value * volscale / 100;
  }

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    SEQ_CONTROL(seq_dev, channel, controller, value);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE | channel);
    SEQ_MIDIOUT(seq_dev, controller);
    SEQ_MIDIOUT(seq_dev, value);
  }
  else
  {
    for (x = 0; x < sinfo[seq_dev].nr_voices; x++)
    {
      if ((voices[x].channel == channel) && (controller == CTL_MAIN_VOLUME))
          SEQ_MAIN_VOLUME(seq_dev, x, value);
    }
  }
  SEQ_DUMPBUF();
}

void patch_change(int patch, int channel)
{
  int x;

#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    SEQ_SET_PATCH(seq_dev, channel, patch);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    SEQ_MIDIOUT(seq_dev, MIDI_PGM_CHANGE | channel);
    SEQ_MIDIOUT(seq_dev, patch);
  }
  else if( synth_ip )
  {
    for (x = 0; x < synth_ip->nr_voices; x++)
    {
      if (((voices[x].channel == -1) && (voices[x].note == -1)) || (voices[x].channel == channel))
      {
        synth_patches[channel] = patch;
        break;
      }
    }
  }
  SEQ_DUMPBUF();
}

void midi_wait( uint32_t wtime )
{
  ioctl(seqfd, SNDCTL_SEQ_SYNC);  // let queue go empty
  SEQ_WAIT_TIME( wtime );   // wait, absolute time
  SEQ_DUMPBUF();
}

// action : mmt_e
void midi_timer(int action)
{
  switch (action)
    {
    case MMT_START:
      SEQ_START_TIMER();
      break;
    case MMT_STOP:
      SEQ_STOP_TIMER();
      break;
    case MMT_CONT:
      SEQ_CONTINUE_TIMER();
      break;
    }
}

static int logscale[32] = {
   0,15, 25,33, 40,45, 50,55, 59,62, 65,68, 70,73, 75,77,
   79,81, 83,85, 87,89, 90,92, 93,94, 95,97, 98,99, 100,100
};

void vol_change(int volume)
{
  int x;

  volume = (volume < 0 ? 0 : (volume > 31 ? 31 : volume));
  volscale = logscale[volume];
#ifdef AWE32_SYNTH_SUPPORT
  if (use_dvt == DVT_AWE32_SYNTH)
  {
    for (x = 0; x < 16; x++)
      SEQ_CONTROL(seq_dev, x, CTL_MAIN_VOLUME, chanvol[x] * volscale / 100);
  }
  else
#endif
  if (use_dvt == DVT_MIDI)
  {
    for (x = 0; x < 16; x++)
    {
      SEQ_MIDIOUT(seq_dev, MIDI_CTL_CHANGE + x);
      SEQ_MIDIOUT(seq_dev, CTL_MAIN_VOLUME);
      SEQ_MIDIOUT(seq_dev, chanvol[x] * volscale / 100);
    }
  }
  else
  {
    volume = volscale;
    volume |= (volume << 8);
    //if (-1 == ioctl(mixfd, SOUND_MIXER_WRITE_SYNTH, &volume))
    //  perror("volume change");
    ioctl(mixfd, SOUND_MIXER_WRITE_SYNTH, &volume);
    ioctl(mixfd, SOUND_MIXER_WRITE_LINE2, &volume);
  }
  SEQ_DUMPBUF();
}


static
void fmload(void)
{
  int x;

  for (x = 0; x < 175; x++)
  {
    fm_sbi[x].key = FM_PATCH;
    fm_sbi[x].device = seq_dev;
    fm_sbi[x].channel = x;
    fm_sbi[x].operators[0] = fm_instruments[x].patchdata[0];
    fm_sbi[x].operators[1] = fm_instruments[x].patchdata[7];
    fm_sbi[x].operators[2] = fm_instruments[x].patchdata[4] + fm_instruments[x].patchdata[5];
    fm_sbi[x].operators[3] = fm_instruments[x].patchdata[11] + fm_instruments[x].patchdata[12];
    fm_sbi[x].operators[4] = fm_instruments[x].patchdata[1];
    fm_sbi[x].operators[5] = fm_instruments[x].patchdata[8];
    fm_sbi[x].operators[6] = fm_instruments[x].patchdata[2];
    fm_sbi[x].operators[7] = fm_instruments[x].patchdata[9];
    fm_sbi[x].operators[8] = fm_instruments[x].patchdata[3];
    fm_sbi[x].operators[9] = fm_instruments[x].patchdata[10];
    fm_sbi[x].operators[10] = fm_instruments[x].patchdata[6];
    fm_sbi[x].operators[11] = fm_instruments[x].patchdata[16];
    fm_sbi[x].operators[12] = fm_instruments[x].patchdata[23];
    fm_sbi[x].operators[13] = fm_instruments[x].patchdata[20] + fm_instruments[x].patchdata[21];
    fm_sbi[x].operators[14] = fm_instruments[x].patchdata[27] + fm_instruments[x].patchdata[28];
    fm_sbi[x].operators[15] = fm_instruments[x].patchdata[17];
    fm_sbi[x].operators[16] = fm_instruments[x].patchdata[24];
    fm_sbi[x].operators[17] = fm_instruments[x].patchdata[18];
    fm_sbi[x].operators[18] = fm_instruments[x].patchdata[25];
    fm_sbi[x].operators[19] = fm_instruments[x].patchdata[19];
    fm_sbi[x].operators[20] = fm_instruments[x].patchdata[26];
    fm_sbi[x].operators[21] = fm_instruments[x].patchdata[22];
    SEQ_WRPATCH(&fm_sbi[x], sizeof(fm_sbi[x]));
  }
}


// Init, load, setup the selected device
void seq_midi_init_setup(int sel_dvt, int dev_type, int port_num)
{
    seq_setup(sel_dvt, dev_type, port_num);

    if (use_dvt == DVT_FM_SYNTH)
    {
        read_wad_genmidi( & genmidi_lump );
        fmload();
    }

    // According to cph, this makes the device really load the instruments
    // Thanks, Colin!!
    cleanup_midi();
  
    seq_setup(sel_dvt, dev_type, port_num);

    if (use_dvt == DVT_FM_SYNTH)
    {
        read_wad_genmidi( & genmidi_lump );
        fmload();
    }
}
