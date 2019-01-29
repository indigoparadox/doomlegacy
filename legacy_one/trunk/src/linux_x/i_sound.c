// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2016 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: i_sound.c,v $
// Revision 1.13  2004/05/13 11:09:38  andyp
// Removed extern int errno references for Linux
//
// Revision 1.12  2004/04/17 12:55:27  hurdler
// now compile with gcc 3.3.3 under Linux
//
// Revision 1.11  2003/07/13 13:16:15  hurdler
// go RC1
//
// Revision 1.10  2003/01/19 21:24:26  bock
// Make sources buildable on FreeBSD 5-CURRENT.
//
// Revision 1.9  2002/07/01 19:59:59  metzgermeister
// *** empty log message ***
//
// Revision 1.8  2001/08/20 20:40:42  metzgermeister
// *** empty log message ***
//
// Revision 1.7  2001/05/16 22:33:35  bock
// Initial FreeBSD support.
//
// Revision 1.6  2000/08/11 19:11:07  metzgermeister
// *** empty log message ***
//
// Revision 1.5  2000/04/30 19:47:38  metzgermeister
// iwad support
//
// Revision 1.4  2000/03/28 16:18:42  linuxcub
// Added a command to the Linux sound-server which sets a master volume.
// Someone needs to check that this isn't too much of a performance drop
// on slow machines. (Works for me).
//
// Added code to the main parts of doomlegacy which uses this command to
// implement volume control for sound effects.
//
// Added code so the (really cool) cd music works for me. The volume didn't
// work for me (with a Teac 532E drive): It always started at max (31) no-
// matter what the setting in the config-file was. The added code "jiggles"
// the volume-control, and now it works for me :-)
// If this code is unacceptable, perhaps another solution is to periodically
// compare the cd_volume.value with an actual value _read_ from the drive.
// Ie. not trusting that calling the ioctl with the correct value actually
// sets the hardware-volume to the requested value. Right now, the ioctl
// is assumed to work perfectly, and the value in cd_volume.value is
// compared periodically with cdvolume.
//
// Updated the spec file, so an updated RPM can easily be built, with
// a minimum of editing. Where can I upload my pre-built (S)RPMS to ?
//
// Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.3  2000/03/12 23:21:10  linuxcub
// Added consvars which hold the filenames and arguments which will be used
// when running the soundserver and musicserver (under Linux). I hope I
// didn't break anything ... Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      System interface for sound.
//
//-----------------------------------------------------------------------------

#include "doomincl.h"
  // stdio, stdlib

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <math.h>

#include <sys/time.h>
#include <sys/types.h>

#if !defined(LINUX) && !defined(SCOOS5) && !defined(_AIX)
#include <sys/filio.h>
#endif

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/msg.h>

// Linux voxware output.
#ifdef LINUX
#ifdef FREEBSD
#include <sys/soundcard.h>
#else
#include <linux/soundcard.h>
#endif
#endif

// SCO OS5 and Unixware OSS sound output
#if defined(SCOOS5) || defined(SCOUW2) || defined(SCOUW7)
#include <sys/soundcard.h>
#endif

// Timer stuff. Experimental.
#include <time.h>
#include <signal.h>

// for IPC between xdoom and musserver
#include <sys/ipc.h>

#include <errno.h>

#include "doomstat.h"
  // nomusic
  // nosoundfx
  // Flags for the -nosound and -nomusic options

#include "i_system.h"
#include "i_sound.h"
#include "s_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "searchp.h"
#include "d_main.h"
#include "z_zone.h"

#include "musserv/musserver.h"


// #define DEBUG_SFX_PIPE

// Master hardware sound volume.
static int hw_sndvolume = 31;

// UNIX hack, to be removed.
#ifdef SNDSERV
#include "sndserv/soundsrv.h"

static FILE *    sndserver = 0;
static uint16_t  handle_cnt = 0;

#elif SNDINTR

// Update all 30 millisecs, approx. 30fps synchronized.
// Linux resolution is allegedly 10 millisecs,
//  scale is microseconds.
#define SOUND_INTERVAL     10000

#else
// None?
#endif

// UNIX hack too, unlikely to be removed.
#ifdef MUSSERV
static int musserver = -1;
static int msg_id = -1;
#endif


// Flag to signal CD audio support to not play a title
//int playing_title;


// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.

// Needed for calling the actual sound output.
#define NUM_CHANNELS            16
#define CHANNEL_NUM_MASK  (NUM_CHANNELS-1)


#ifndef SNDSERV

#define SAMPLECOUNT             1024
#define SAMPLERATE              11025   // Hz
#define SAMPLESIZE              2       // 16bit
// It is 2 for 16bit, and 2 for two channels.
#define BUFMUL                  4
#define MIXBUFFERSIZE           (SAMPLECOUNT*BUFMUL)


// The actual output device and a flag for using 8bit samples.
static int audio_fd;
static byte audio_8bit_flag;



typedef struct {
    // the channel data, current position
    byte *  data;  // NULL=inactive
    // the channel data end pointer
    byte *  data_end;
    // the channel step amount
    unsigned int step;
    // 0.16 bit remainder of last step
    unsigned int remainder;
    // volumes
    int  left_volume, right_volume;
    // the channel volume lookup, modifed by master volume
    int  * left_vol_tab, * right_vol_tab;
    // Time/gametic that the channel started playing,
    //  used to determine oldest, which automatically has lowest priority.
    // In case number of active sounds exceeds available channels.
    int  start_time;
    // The channel handle, determined on registration,
    //  might be used to unregister/stop/modify.
    // Lowest bits are the channel num.
    int  handle;
    // SFX id of the playing sound effect.
    // Used to catch duplicates (like chainsaw).
    uint16_t  id;
   
#ifdef SURROUND_SOUND
    byte  invert_right;
#endif

} channel_info_t;

static channel_info_t   channel[NUM_CHANNELS];

// The global mixing buffer.
// Basically, samples from all active internal channels
//  are modifed and added, and stored in the buffer
//  that is submitted to the audio device.
static int16_t  mixbuffer[MIXBUFFERSIZE];


// Pitch to stepping lookup, unused.
static int steptable[256];

// Volume lookups.
static int vol_lookup[128][256];

#endif

#ifndef SNDSERV
//
// Safe ioctl, convenience.
//
void myioctl(int fd, int command, int *arg)
{
    static byte  ioctl_err_count = 0;
    static byte  ioctl_err_off = 0;
    int rc;

    if( ioctl_err_off )
    {
        ioctl_err_off--;
        return;
    }

    rc = ioctl(fd, command, arg);
    if (rc < 0)
    {
        GenPrintf(EMSG_error, "ioctl(dsp,%d,arg) failed\n", command);
        GenPrintf(EMSG_error, "errno=%d\n", errno);
        // [WDJ] No unnecessary fatal exits, let the player savegame.
        if( ioctl_err_count < 254 )
            ioctl_err_count++;
        if( ioctl_err_count > 10 )
            ioctl_err_off = 20;
       
//        exit(-1);
    }
}
#endif


// Interface
// This function loads the sound data from the WAD lump,
//  for single sound.
//
void I_GetSfx(sfxinfo_t * sfx)
{
    byte *dssfx;
    int size;

    
    S_GetSfxLump( sfx ); // lump to sfx
    // Linked sounds will reuse the data.
    // Can set data to NULL, but cannot change its format.
    
    dssfx = (byte *) sfx->data;
    if( ! dssfx )  return;

    // Sound data header format.
    // 0,1: 03
    // 2,3: sample rate (11,2B)=11025, (56,22)=22050
    // 4,5: number of samples
    // 6,7: 00
    size = sfx->length;
    if( size <= 8 )
    {
        size = 8;
        GenPrintf( EMSG_warn, "GetSfx, short sound: %s\n", sfx->name );
    }

#ifdef SNDSERV
    // write data to llsndserv 19990201 by Kin
    if (sndserver)
    {
        server_load_sound_t  sls;
        // Send sound data to server.
        sls.flags = sfx->flags;
        // The sound server does not need padded sound, and if it did
        // it could more easily do that itself.
        sls.snd_len = size - 8;
        // [WDJ] No longer send volume with load, as it interferes with
        // the automated volume update.
        sls.id = sfx - S_sfx;

#ifdef  DEBUG_SFX_PIPE
        GenPrintf( EMSG_debug," Command L:  Sfx  size=%x\n", sfx->length );
        GenPrintf( EMSG_debug," Load sound, sfx=%i, snd_len=%i  flagx=%x\n ", sls.id, sls.snd_len, sls.flags );
#endif
        // sfx data loaded to sound server at sfx id.
        fputc('l', sndserver);
        fwrite((byte*)&sls, sizeof(sls), 1, sndserver);  // sfx id, flags, size
        fwrite(&dssfx[8], 1, sls.snd_len, sndserver);
        fflush(sndserver);
    }
#else
#ifdef  HAVE_ALLEGRO   
    // convert raw data and header from Doom sfx to a SAMPLE for Allegro
    // Linked sound will already be padded.
    int sampsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;
    int reqsize = sampsize + 8;
    if( reqsize > size )
    {
        // Only reallocate when necessary.
        byte *paddedsfx = (byte *) Z_Malloc(reqsize, PU_STATIC, 0);
        memcpy(paddedsfx, dssfx, size);
        for (i = size; i < reqsize; i++)
            paddedsfx[i] = 128;
        sfx->data = (void *) paddedsfx;
        sfx->length = reqsize;
        // Remove the cached lump.
        Z_Free(dssfx);
        dssfx = paddedsfx;
    }
    *((uint32_t *) dssfx) = sampsize;
#endif
#endif
}

void I_FreeSfx(sfxinfo_t * sfx)
{
    // normal free
}


#ifndef SNDSERV
//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
//  vol : volume, 0..255
//  sep : separation, +/- 127, SURROUND_SEP special operation
static
int addsfx_ch(int sfxid, int vol, int step, int sep)
{
    channel_info_t * chp, * chp2;
    int i, oldest;
    int slot;
    int leftvol, rightvol;

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (S_sfx[sfxid].flags & SFX_single)
    {
        // Loop all channels, check.
        for( chp2 = &channel[0]; chp2 < &channel[cv_numChannels.value]; chp2++ )
        {
            // Active, and using the same SFX?
            if (chp2->data && (chp2->id == sfxid))
            {
                if( S_sfx[sfxid].flags & SFX_id_fin )
                    return chp2->handle;  // already have one
                // Kill, Reset.
                chp2->data = NULL;  // close existing channel
                break;
            }
        }
    }

    // Find inactive channel, or oldest channel.
    chp = &channel[0];  // default
    oldest = MAXINT;
    for ( chp2 = &channel[0]; chp2 < &channel[cv_numChannels.value]; chp2++ )
    {
        if( chp2->data == NULL )
        {
            chp = chp2;  // Inactive channel
            break;
        }
        if( chp2->start_time < oldest )
        {
            chp = chp2;  // older channel
            oldest = chp->start_time;
        }
    }

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    chp->id = sfxid;
    // Set pointer to raw data.
    chp->data = & S_sfx[sfxid].data[8];  // after header
    // Set pointer to end of raw data.
    chp->data_end = chp->data + S_sfx[sfxid].length - 8; // without header

    // Set stepping
    // Kinda getting the impression this is never used.
    chp->step = step;
    chp->remainder = 0;
    // Should be gametic, I presume.
    chp->start_time = gametic;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.

#ifdef SURROUND_SOUND
    chp->invert_right = 0;
    if( sep == SURROUND_SEP )
    {
        // Use a normal sound data for the left channel (with pan left)
        // and an inverted sound data for the right channel (with pan right)
        leftvol = rightvol = (vol * (224 * 224)) >> 16;  // slight reduction going through panning
        chp->invert_right = 1;  // invert right channel
    }
    else
#endif
    {
        // Separation, that is, orientation/stereo.
        // sep : +/- 127, <0 is left, >0 is right
        sep += 129;  // 129 +/- 127 ; ( 1 - 256 )
        leftvol = vol - ((vol * sep * sep) >> 16);
        sep = 258 - sep;  // 129 +/- 127
        rightvol = vol - ((vol * sep * sep) >> 16);
    }

    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
    {
        I_SoftError("rightvol out of bounds\n");
        rightvol = ( rightvol < 0 ) ? 0 : 127;
    }

    if (leftvol < 0 || leftvol > 127)
    {
        I_SoftError("leftvol out of bounds\n");
        leftvol = ( leftvol < 0 ) ? 0 : 127;
    }

    // Get the proper lookup table for this volume level.
    chp->left_volume = leftvol;
    chp->left_vol_tab = &vol_lookup[(leftvol * hw_sndvolume / 31)][0];
    chp->right_volume = rightvol;
    chp->right_vol_tab = &vol_lookup[(rightvol * hw_sndvolume / 31)][0];

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    chp->handle = slot | ((chp->handle + NUM_CHANNELS) & ~CHANNEL_NUM_MASK);
    return chp->handle;
}
#endif

#ifndef SNDSERV
//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the old DPMS based DOS version, this
// were simply dummies in the Linux version.
// See soundserver initdata().
//
void I_SetChannels()
{
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    //  the mixing process. 
    int i;
    int j;

   
    int *steptablemid = steptable + 128;

    memset( channel, 0, sizeof(channel) );
   
    // This table provides step widths for pitch parameters.
    // I fail to see that this is currently used.
    for (i = -128; i < 128; i++)
        steptablemid[i] = (int) (pow(2.0, (i / 64.0)) * 65536.0);

    // Generates volume lookup tables
    //  which also turn the unsigned samples
    //  into signed samples.
    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 256; j++)
        {
            if (!audio_8bit_flag)
                vol_lookup[i][j] = (i * (j - 128) * 256) / 127;
            else
                vol_lookup[i][j] = (i * (j - 128) * 256) / 127 * 4;
        }
    }
}
#endif


// new_volume : 0..31
void I_SetSfxVolume(int volume)
{
    // Identical to DOS.
    // Basically, this should propagate the menu/config file setting
    //  to the state variable used in the mixing.

#ifdef  DEBUG_SFX_PIPE
        GenPrintf( EMSG_debug," Testing:  volume=%i\n", hw_sndvolume );
#endif
#ifdef SNDSERV
    hw_sndvolume = volume;

    if (sndserver)
    {
#ifdef  DEBUG_SFX_PIPE
        GenPrintf( EMSG_debug," Command V:  volume=%i\n", hw_sndvolume );
#endif
        fputc('v', sndserver);
        fputc((byte) hw_sndvolume, sndserver);
        fflush(sndserver);
    }
#else

    if( volume == hw_sndvolume )
       return;

    if( volume > 31 )  volume = 31;
    hw_sndvolume = volume;

    // Update existing channel volumes.
    register channel_info_t * chp;
    for( chp = &channel[0]; chp < &channel[cv_numChannels.value]; chp++ )
    {
        if( chp->data )
        {
            chp->left_vol_tab = &volume_lookup[(chp->left_volume * volume)/31][0];
            chp->right_vol_tab = &volume_lookup[(chp->right_volume * volume)/31][0];
        }
    }
}
#endif
}



//
// Starting a sound means adding it
//  to the current list of active sounds
//  in the internal channels.
// As the SFX info struct contains
//  e.g. a pointer to the raw data,
//  it is ignored.
// As our sound handling does not handle
//  priority, it is ignored.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
// Starts a sound in a particular sound channel.
//  vol : 0..255
// Return a handle to the sound.
int I_StartSound ( sfxid_t sfxid, int vol, int sep, int pitch, int priority )
{
    // UNUSED
    priority = 0;
    vol = vol >> 4;     // xdoom only accept 0-15 19990124 by Kin

    if (nosoundfx)
        return 0;

#ifdef SNDSERV
    if (sndserver)
    {
        server_play_sound_t  sps;

        sps.sfxid = sfxid;
        sps.vol = vol;
        sps.pitch = pitch;
        sps.sep = sep;
        sps.handle = handle_cnt++;

#ifdef  DEBUG_SFX_PIPE
        GenPrintf( EMSG_debug," Command P:" );
        GenPrintf( EMSG_debug," Play sound, sfx=%i, vol=%i, pitch=%i, sep=%i, handle=%i\n ", sps.sfxid, sps.vol, sps.pitch, sps.sep, sps.handle );
#endif
        // play sound
        fputc('p', sndserver);
        fwrite((byte*)&sps, sizeof(sps), 1, sndserver);
        fflush(sndserver);
        return sps.handle;
    }
    return 0;
#else
    // Debug.
    //GenPrintf(EMSG_debug, "starting sound %d", id );

    // Returns a handle.
    int handle = addsfx_ch(id, vol, steptable[pitch], sep);

    //GenPrintf(EMSG_debug, "/handle is %d\n", id );

    return handle;
#endif
}

// You need the handle returned by StartSound.
void I_StopSound(int handle)
{
#ifdef SNDSERV
    uint16_t  handle16 = handle;

#ifdef  DEBUG_SFX_PIPE
    GenPrintf( EMSG_debug," Command S:  Stop  handle=%i\n", handle );
#endif
   
    // Send stop sound.
    fputc('s', sndserver);
    fwrite(&handle16, sizeof(uint16_t), 1, sndserver);  // handle
    fflush(sndserver);
#else
    int slot = handle & CHANNEL_NUM_MASK;
    if (channel[slot].handle == handle)
    {
        channel[i].data = NULL;
    }
#endif
}

int I_SoundIsPlaying(int handle)
{
#ifdef SNDSERV
    return (handle_cnt - ((uint16_t)handle)) < 8;  // guess
#else
    int slot = handle & CHANNEL_NUM_MASK;
    if (channel[slot].handle == handle)
    {
#if 1
        return ( channel[chan].data != NULL );

#else
        // old code
            return 1;
#endif
    }
    return 0;
#endif
}



#ifdef SNDINTR
// Get the interrupt. Set duration in millisecs.
int I_SoundSetTimer(int duration_of_tick);
void I_SoundDelTimer(void);
#endif

#ifndef SNDSERV
// A quick hack to establish a protocol between
// synchronous mix buffer updates and asynchronous
// audio writes. Probably redundant with gametic.
volatile static int mix_cnt = 0;

//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the global
//  mixbuffer, clamping it to the allowed range,
//  and sets up everything for transferring the
//  contents of the mixbuffer to the (two)
//  hardware channels (left and right, that is).
//
void I_UpdateSound(void)
{
    // Debug. Count buffer misses with interrupt.
    static int misses = 0;

    // Flag. Will be set if the mixing buffer really gets updated.
    byte updated = 0;

    channel_info_t * chp;

    // Mix current sound data.
    // Data, from raw sound, for right and left.
    register unsigned int sample;
    register int dl, dr;
    uint16_t  sdl, sdr;

    // Pointers in global mixbuffer, left, right, end.
    int16_t * leftout;
    int16_t * rightout;
    int16_t * leftend;
    byte * bothout;

    // Step in mixbuffer, left and right, thus two.
    int step;


    if (dedicated)
        return;

    // Left and right channel
    //  are in global mixbuffer, alternating.
    leftout = mixbuffer;
    rightout = mixbuffer + 1;
    bothout = (byte *) mixbuffer;
    step = 2;

    // Determine end, for left channel only
    //  (right channel is implicit).
    leftend = mixbuffer + SAMPLECOUNT * step;

    // Mix sounds into the mixing buffer.
    // Loop over step*SAMPLECOUNT.
    while (leftout != leftend)
    {
        // Reset left/right value. 
        dl = 0;
        dr = 0;

        // Love thy L2 chache - made this a loop.
        // Now more channels could be set at compile time
        //  as well. Thus loop those  channels.
        for( chp = &channel[0]; chp < &channel[cv_numChannels.value]; chp++ )
        {
            // Check channel, if active.
            if (chp->data)
            {
                // we are updating the mixer buffer, set flag
                updated = 1;
                // Get the raw data from the channel. 
                sample = * chp->data;
                // Add left and right part for this channel (sound)
                //  to the current data.
                // Adjust volume accordingly.
                dl += chp->left_vol_tab[sample];
#ifdef SURROUND_SOUND
                if( chp->invert_right )
                  dr -= chp->right_vol_tab[sample];
                else
                  dr += chp->right_vol_tab[sample];
#else
                dr += chp->right_vol_tab[sample];
#endif
                // Increment fixed point index
                chp->remainder += chp->step;
                // MSB is next sample
                chp->data += chp->remainder >> 16;
                // Keep the fractional index part
                chp->remainder &= 0xFFFF;

                // Check whether we are done.
                if (chp->data >= chp->data_end)
                    chp->data = NULL;
            }
        }

        // Clamp to range. Left hardware channel.
        // Has been char instead of int16.
        // if (dl > 127) *leftout = 127;
        // else if (dl < -128) *leftout = -128;
        // else *leftout = dl;

        if (!audio_8bit_flag)
        {
            if (dl > 0x7fff)
                *leftout = 0x7fff;
            else if (dl < -0x8000)
                *leftout = -0x8000;
            else
                *leftout = dl;

            // Same for right hardware channel.
            if (dr > 0x7fff)
                *rightout = 0x7fff;
            else if (dr < -0x8000)
                *rightout = -0x8000;
            else
                *rightout = dr;
        }
        else
        {
            if (dl > 0x7fff)
                dl = 0x7fff;
            else if (dl < -0x8000)
                dl = -0x8000;
            sdl = dl ^ 0xfff8000;

            if (dr > 0x7fff)
                dr = 0x7fff;
            else if (dr < -0x8000)
                dr = -0x8000;
            sdr = dr ^ 0xfff8000;

            *bothout++ = (((sdr + sdl) / 2) >> 8);
        }

        // Increment current pointers in mixbuffer.
        leftout += step;
        rightout += step;
    }

    if (updated)
    {
        // Debug check.
        if (mix_cnt)
        {
            misses += mix_cnt;
            mix_cnt = 0;
        }

        if (misses > 10)
        {
            GenPrintf(EMSG_warn, "I_SoundUpdate: missed 10 buffer writes\n");
            misses = 0;
        }

        // Increment mix_cnt for update.
        mix_cnt++;
    }
}
#endif

#ifdef SNDSERV
// [WDJ] Fix this in d_main.c
void I_SubmitSound(void)
{
}
#else
// This is used to write out the mixbuffer
//  during each game loop update.
//
void I_SubmitSound(void)
{
    if (dedicated)
        return;

    // Write it to DSP device.
    if (mix_cnt)
    {
        if (!audio_8bit_flag)
            write(audio_fd, mixbuffer, SAMPLECOUNT * BUFMUL);
        else
            write(audio_fd, mixbuffer, SAMPLECOUNT);
        mix_cnt = 0;
    }
}
#endif



// Interface
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    // I fail too see that this is used.
    // Would be using the handle to identify
    //  on which channel the sound might be active,
    //  and resetting the channel parameters.

    // UNUSED.
    handle = vol = sep = pitch = 0;
}


static
void LX_ShutdownSound(void)
{
#ifdef SNDSERV
    if (sndserver)
    {
#ifdef  DEBUG_SFX_PIPE
        GenPrintf( EMSG_debug," Command Q:\n" );
#endif
        // Send a "quit" command.
        fputc('q', sndserver);
        fflush(sndserver);
    }

#else

    // Wait till all pending sounds are finished.
    int done = 0;
    int i;

    if (nosoundfx)
        return;

#ifdef SNDINTR
    I_SoundDelTimer();
#endif

    while (!done)
    {
        for (i = 0; i < cv_numChannels.value; i++)
           if( channel[i].data ) break;  // any busy channel
        if (i == cv_numChannels.value)
            done++;
        else
        {
            I_UpdateSound();
            I_SubmitSound();
        }
    }

    // Cleaning up -releasing the DSP device.
    close(audio_fd);
#endif

    // Done.
    return;
}


static
void LX_InitSound()
{
#ifdef SNDSERV
    char buffer[2048];
    char *fn_snd;

    fn_snd = searchpath(cv_sndserver_cmd.string);

    // start sound process
    if (!access(fn_snd, X_OK))
    {
        sprintf(buffer, "%s %s", fn_snd, cv_sndserver_arg.string);
        sndserver = popen(buffer, "w");
#ifdef  DEBUG_SFX_PIPE
        GenPrintf( EMSG_debug," Started Sound Server:\n" );
#endif
    }
    else
        GenPrintf(EMSG_error, "Could not start sound server [%s]\n", fn_snd);
#else

    int i;

    if (nosoundfx)
        return;

    // Secure and configure sound device first.
    GenPrintf(EMSG_info, "LX_InitSound: ");

    audio_fd = open("/dev/dsp", O_WRONLY);
    if (audio_fd < 0)
    {
        GenPrintf(EMSG_error, "Could not open /dev/dsp\n");
        nosoundfx++;
        return;
    }

#ifdef SOUND_RESET
    myioctl(audio_fd, SNDCTL_DSP_RESET, 0);
#endif

    audio_8bit_flag = 1;  // default
    if (getenv("DOOM_SOUND_SAMPLEBITS") == NULL)
    {
        myioctl(audio_fd, SNDCTL_DSP_GETFMTS, &i);
        if (i &= AFMT_S16_LE)
        {
            audio_8bit_flag = 0;
            myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
            i = 11 | (2 << 16);
            myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
            i = 1;
            myioctl(audio_fd, SNDCTL_DSP_STEREO, &i);
        }
    }

    if( audio_8bit_flag )  // default
    {
        i = AFMT_U8;
        myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
        i = 10 | (2 << 16);
        myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
    }

    i = SAMPLERATE;
    myioctl(audio_fd, SNDCTL_DSP_SPEED, &i);

    GenPrintf(EMSG_info, " configured %dbit audio device\n", (audio_8bit_flag) ? 8 : 16);

#ifdef SNDINTR
    GenPrintf(EMSG_info, "I_SoundSetTimer: %d microsecs\n", SOUND_INTERVAL);
    I_SoundSetTimer(SOUND_INTERVAL);
#endif

    // Initialize external data (all sounds) at start, keep static.
    GenPrintf(EMSG_info, "LX_InitSound: ");

    // Do we have a sound lump for the chaingun?
    if (W_CheckNumForName("dschgun") == -1)
    {
        // No, so link it to the pistol sound
        //S_sfx[sfx_chgun].link = &S_sfx[sfx_pistol];
        //S_sfx[sfx_chgun].pitch = 150;
        //S_sfx[sfx_chgun].volume = 0;
        //S_sfx[sfx_chgun].data = 0;
        GenPrintf(EMSG_info, "linking chaingun sound to pistol sound,");
    }
    else
    {
        GenPrintf(EMSG_info, "found chaingun sound,");
    }

    GenPrintf(EMSG_info, " pre-cached all sound data\n");

    // Now initialize mixbuffer with zero.
    for (i = 0; i < MIXBUFFERSIZE; i++)
        mixbuffer[i] = 0;

    // Finished initialization.
    GenPrintf(EMSG_info, "LX_InitSound: sound module ready\n");

#endif
}


// --- Music
//#define DEBUG_MUSSERV

#ifdef MUSSERV
mus_msg_t  msg_buffer;

void send_val_musserver( char command, char sub_command, int val )
{
    if( msg_id < 0 )  return;

    msg_buffer.mtype = 5;
    memset(msg_buffer.mtext, 0, MUS_MSG_MTEXT_LENGTH);
    snprintf(msg_buffer.mtext, MUS_MSG_MTEXT_LENGTH-1, "%c%c%i",
             command, sub_command, val);
    msg_buffer.mtext[MUS_MSG_MTEXT_LENGTH-1] = 0;
#ifdef  DEBUG_MUSSERV
    GenPrintf( EMSG_debug, "Send musserver: %s\n", msg_buffer.mtext );
#endif
    msgsnd(msg_id, MSGBUF(msg_buffer), 12, IPC_NOWAIT);
    usleep(2);  // just enough for musserver to respond promptly.
}
#endif

// Music volume may be set before calling LX_InitMusic.
static int music_volume = 0;

//
// MUSIC API.
// Music done now, we'll use Michael Heasley's musserver.
//
static
void LX_InitMusic(void)
{
#ifdef MUSSERV
    char buffer[MAX_WADPATH];
    char *fn_mus;

    fn_mus = searchpath(cv_musserver_cmd.string);

    // Try to start the music server process.
    if ( access(fn_mus, X_OK) < 0)
    {
        GenPrintf(EMSG_error, "Could not find music server [%s]\n", fn_mus);
        return;
    }

    // [WDJ] Use IPC for settings, not command line.
    snprintf(buffer, MAX_WADPATH-1, "%s %s &", fn_mus, cv_musserver_arg.string);
    buffer[MAX_WADPATH-1] = 0;

    GenPrintf( EMSG_info, "Starting music server [%s]\n", buffer);
    // Sys call "system()"  seems to work, and does not need \n.
    // It returns 0 on success.
    musserver = system(buffer);
    if( musserver < 0 )
    {
        GenPrintf( EMSG_error, "Could not start music server [%s]\n", fn_mus);
        return;
    }

    msg_id = msgget(MUSSERVER_MSG_KEY, IPC_CREAT | 0777);
    if( verbose > 1 )
        GenPrintf( EMSG_info, "Started Musicserver = %i, IPC = %i\n", musserver, msg_id );
    send_val_musserver( 'v', ' ', music_volume );
    // [WDJ] Starting with system() gives the process a PPID of 1, which is Init.
    // When DoomLegacy is killed, it is not detected by musserver.
    send_val_musserver( 'I', ' ', getpid() ); // our pid
    // Send this again because it was too early at configure time.
    I_SetMusicOption();
#endif
}

void LX_ShutdownMusic(void)
{
    if (nomusic)
        return;

#ifdef MUSSERV
    // [WDJ] It is a race between the quit command and the queue destruction.
    // Rely upon one or the other.
#if 1
    // send a "quit" command.
    send_val_musserver( 'Q', 'Q', 0 );
#else
    if (musserver > -1)
    {
        // Close the queue.
        if (msg_id != -1)
            msgctl(msg_id, IPC_RMID, (struct msqid_ds *) NULL);
    }
#endif
#endif
}


// MUSIC API
void I_SetMusicVolume(int volume)
{
    // Internal state variable.
    music_volume = volume;
    // Now set volume on output device.
    // Whatever( snd_MusciVolume );

    if (nomusic)
        return;

#ifdef MUSSERV
    send_val_musserver( 'v', ' ', volume );
#endif
}

// MUSIC API

char * mus_ipc_opt_tab[] = {
  "-dd", // Default
  "-da", // Search 1
  "-db", // Search 2
  "-dc", // Search 3
  "-dM", // Midi
  "-dT", // TiMidity
  "-dL", // FluidSynth
  "-dE", // Ext Midi
  "-dS", // Synth
  "-dF", // FM Synth
  "-dA", // Awe32 Synth
  "-dg", // Dev6
  "-dh", // Dev7
  "-dj", // Dev8
  "-dk"  // Dev9
};

void I_SetMusicOption(void)
{
    byte bi = cv_musserver_opt.value;

    if( msg_id < 0 )  return;
    if( bi > 16 )  return;

    msg_buffer.mtype = 6;
    memset(msg_buffer.mtext, 0, MUS_MSG_MTEXT_LENGTH);
    snprintf(msg_buffer.mtext, MUS_MSG_MTEXT_LENGTH-1, "O%s", mus_ipc_opt_tab[bi] );
    msg_buffer.mtext[MUS_MSG_MTEXT_LENGTH-1] = 0;
#ifdef  DEBUG_MUSSERV
    GenPrintf( EMSG_debug, "Send musserver option: %s\n", msg_buffer.mtext );
#endif
    msg_buffer.mtext[MUS_MSG_MTEXT_LENGTH-1] = 0;
    msgsnd(msg_id, MSGBUF(msg_buffer), MUS_MSG_MTEXT_LENGTH, IPC_NOWAIT);
}



static byte music_looping = 0;
static int music_dies = -1;



void I_PauseSong(int handle)
{
    if (nomusic)
        return;

#ifdef MUSSERV
    send_val_musserver( 'P', 'P', 1 );
#endif
    handle = 0;  // UNUSED
}

void I_ResumeSong(int handle)
{
    if (nomusic)
        return;

#ifdef MUSSERV
    send_val_musserver( 'P', 'R', 0 );
#endif
    handle = 0;  // UNUSED
}

void I_StopSong(int handle)
{
    if (nomusic)
        return;

#ifdef MUSSERV
    send_val_musserver( 'X', 'X', 0 );
#endif
    handle = 0; // UNUSED.
    music_looping = 0;
    music_dies = 0;
}

void I_UnRegisterSong(int handle)
{
    handle = 0; // UNUSED.
}


#ifdef MUSSERV
// Information for ports with music servers.
//  name : name of song
// Return handle
int I_PlayServerSong( char * name, lumpnum_t lumpnum, byte looping )
{
    if (nomusic)
    {
        return 1;
    }

    music_dies = gametic + (TICRATE * 30);

    music_looping = looping;

    if (msg_id != -1)
    {
        static  byte sent_genmidi = 0;
        wadfile_t * wadp;
       
        msg_buffer.mtype = 6;
        if( sent_genmidi == 0 )
        {
            sent_genmidi = 1;
            // Music server needs the GENMIDI lump, which may depend
            // upon the IWAD and PWAD order.
            lumpnum_t  genmidi_lumpnum = W_GetNumForName( "GENMIDI" );
            wadp = lumpnum_to_wad( genmidi_lumpnum );
            if( wadp )
            {
                memset(msg_buffer.mtext, 0, MUS_MSG_MTEXT_LENGTH);
                snprintf(msg_buffer.mtext, MUS_MSG_MTEXT_LENGTH-1, "W%s", wadp->filename);
                msg_buffer.mtext[MUS_MSG_MTEXT_LENGTH-1] = 0;
#ifdef  DEBUG_MUSSERV
                GenPrintf( EMSG_debug, "Send musserver wad: %s\n", msg_buffer.mtext );
#endif
                msgsnd(msg_id, MSGBUF(msg_buffer), MUS_MSG_MTEXT_LENGTH, IPC_NOWAIT);
            }
            // Sending genmidi lumpnum to musserver.
            send_val_musserver( 'G', ' ', LUMPNUM(genmidi_lumpnum) );
        }
        // Send song name to musserver
        memset(msg_buffer.mtext, 0, MUS_MSG_MTEXT_LENGTH);
        sprintf(msg_buffer.mtext, "D %s", name);
#ifdef  DEBUG_MUSSERV
        GenPrintf( EMSG_debug, "Send musserver song: %s\n", msg_buffer.mtext );
#endif
        msgsnd(msg_id, MSGBUF(msg_buffer), 12, IPC_NOWAIT);
        // Song info
        wadp = lumpnum_to_wad( lumpnum );
        if( wadp )
        {
            // Send song wad information to server
            memset(msg_buffer.mtext, 0, MUS_MSG_MTEXT_LENGTH);
            snprintf(msg_buffer.mtext, MUS_MSG_MTEXT_LENGTH-1, "W%s", wadp->filename);
            msg_buffer.mtext[MUS_MSG_MTEXT_LENGTH-1] = 0;
#ifdef  DEBUG_MUSSERV
            GenPrintf( EMSG_debug, "Send musserver wad: %s\n", msg_buffer.mtext );
#endif
            msgsnd(msg_id, MSGBUF(msg_buffer), MUS_MSG_MTEXT_LENGTH, IPC_NOWAIT);
        }
        // Sending song lumpnum to musserver.
        send_val_musserver( 'S', (looping?'C':' '), LUMPNUM(lumpnum) );
    }
    return 1;
}


#else
// not MUSSERV

// Interface
int I_RegisterSong( void* data, int len )
{
    if (nomusic)
    {
        return 1;
    }

    data = NULL;
    return 1;
}

// Interface
void I_PlaySong(int handle, int looping)
{
    music_dies = gametic + (TICRATE * 30);

    if (nomusic)
        return;

    music_looping = looping;
    handle = 0;
}
#endif


#if 0
// Disabled call, no interface.
// Is the song playing?
int I_QrySongPlaying(int handle)
{
    handle = 0;  // UNUSED
    return music_looping || (music_dies > gametic);
}
#endif


//--- Sound system Interface
// Interface, Start sound system.
void I_StartupSound()
{
   if( dedicated )
       return;

   if(! nosoundfx)
       LX_InitSound();
   if(! nomusic )
       LX_InitMusic();
}

// Interface, Shutdown sound system.
void I_ShutdownSound(void)
{
   LX_ShutdownSound();
   LX_ShutdownMusic();
}


#ifdef SNDINTR
//
// Experimental stuff.
// A Linux timer interrupt, for asynchronous
//  sound output.
// I ripped this out of the Timer class in
//  our Difference Engine, including a few
//  SUN remains...
//  
#ifdef sun
typedef sigset_t tSigSet;
#else
typedef int tSigSet;
#endif

// We might use SIGVTALRM and ITIMER_VIRTUAL, if the process
//  time independend timer happens to get lost due to heavy load.
// SIGALRM and ITIMER_REAL doesn't really work well.
// There are issues with profiling as well.

//static int /*__itimer_which*/  itimer = ITIMER_REAL;
static int /*__itimer_which*/ itimer = ITIMER_VIRTUAL;

//static int sig = SIGALRM;
static int sig = SIGVTALRM;

// Interrupt handler.
static void I_HandleSoundTimer(int ignore)
{
    // Debug.
    //GenPrintf(EMSG_debug, "%c", '+' ); fflush( stderr );

    // Feed sound device if necesary.
    if (mix_cnt)
    {
        // See I_SubmitSound().
        // Write it to DSP device.
        if (!audio_8bit_flag)
            write(audio_fd, mixbuffer, SAMPLECOUNT * BUFMUL);
        else
            write(audio_fd, mixbuffer, SAMPLECOUNT);

        // Reset flag counter.
        mix_cnt = 0;
    }
    else
        return;

    // UNUSED, but required.
    ignore = 0;
    return;
}

// Get the interrupt. Set duration in millisecs.
static int I_SoundSetTimer(int duration_of_tick)
{
    // Needed for gametick clockwork.
    struct itimerval value;
    struct itimerval ovalue;
    struct sigaction act;
    struct sigaction oact;

    int res;

    // This sets to SA_ONESHOT and SA_NOMASK, thus we can not use it.
    //     signal( _sig, handle_SIG_TICK );

    // Now we have to change this attribute for repeated calls.
    act.sa_handler = I_HandleSoundTimer;
#ifndef sun
    //ac  t.sa_mask = _sig;
#endif
    act.sa_flags = SA_RESTART;

    sigaction(sig, &act, &oact);

    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = duration_of_tick;
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = duration_of_tick;

    // Error is -1.
    res = setitimer(itimer, &value, &ovalue);

    // Debug.
    if (res == -1)
        GenPrintf(EMSG_debug, "I_SoundSetTimer: interrupt n.a.\n");

    return res;
}

// Remove the interrupt. Set duration to zero.
static void I_SoundDelTimer()
{
    // Debug.
    if (I_SoundSetTimer(0) == -1)
        GenPrintf(EMSG_debug, "I_SoundDelTimer: failed to remove interrupt. Doh!\n");
}
#endif

#ifdef FMOD_SONG
//Hurdler: TODO
void I_StartFMODSong()
{
    CONS_Printf("I_StartFMODSong: Not yet supported under Linux.\n");
}

void I_StopFMODSong()
{
    CONS_Printf("I_StopFMODSong: Not yet supported under Linux.\n");
}
void I_SetFMODVolume(int volume)
{
    CONS_Printf("I_SetFMODVolume: Not yet supported under Linux.\n");
}
#endif
