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


// UNIX hack, to be removed.
#ifdef SNDSERV
FILE *sndserver = 0;
int sndcnt = 0;                 // sfx serial no. 19990201 by Kin
#elif SNDINTR

// Update all 30 millisecs, approx. 30fps synchronized.
// Linux resolution is allegedly 10 millisecs,
//  scale is microseconds.
#define SOUND_INTERVAL     10000

// Get the interrupt. Set duration in millisecs.
int I_SoundSetTimer(int duration_of_tick);
void I_SoundDelTimer(void);
#else
// None?
#endif

// UNIX hack too, unlikely to be removed.
#ifdef MUSSERV
int musserver = -1;
int msg_id = -1;
#endif


// Flag to signal CD audio support to not play a title
//int playing_title;

// A quick hack to establish a protocol between
// synchronous mix buffer updates and asynchronous
// audio writes. Probably redundant with gametic.
volatile static int flag = 0;

// The number of internal mixing channels,
//  the samples calculated for each mixing step,
//  the size of the 16bit, 2 hardware channel (stereo)
//  mixing buffer, and the samplerate of the raw data.

// Needed for calling the actual sound output.
#define SAMPLECOUNT             1024
#define NUM_CHANNELS            16
#define CHANNEL_NUM_MASK  (NUM_CHANNELS-1)
// It is 2 for 16bit, and 2 for two channels.
#define BUFMUL                  4
#define MIXBUFFERSIZE           (SAMPLECOUNT*BUFMUL)

#define SAMPLERATE              11025   // Hz
#define SAMPLESIZE              2       // 16bit


// The actual output device and a flag for using 8bit samples.
int audio_fd;
int audio_8bit_flag;

// The global mixing buffer.
// Basically, samples from all active internal channels
//  are modifed and added, and stored in the buffer
//  that is submitted to the audio device.
int16_t  mixbuffer[MIXBUFFERSIZE];

// The channel step amount...
unsigned int channelstep[NUM_CHANNELS];
// ... and a 0.16 bit remainder of last step.
unsigned int channelstepremainder[NUM_CHANNELS];

// The channel data pointers, start and end.
unsigned char *channels[NUM_CHANNELS];
unsigned char *channelsend[NUM_CHANNELS];

// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
int channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
// Lowest bits are the channel num.
int channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
int channelids[NUM_CHANNELS];

// Pitch to stepping lookup, unused.
int steptable[256];

// Volume lookups.
int vol_lookup[128 * 256];

// Hardware left and right channel volume lookup.
int *channelleftvol_lookup[NUM_CHANNELS];
int *channelrightvol_lookup[NUM_CHANNELS];

// master hardware sound volume
int hw_sndvolume = 31;

//
// Safe ioctl, convenience.
//
void myioctl(int fd, int command, int *arg)
{
    int rc;

    rc = ioctl(fd, command, arg);
    if (rc < 0)
    {
        GenPrintf(EMSG_error, "ioctl(dsp,%d,arg) failed\n", command);
        GenPrintf(EMSG_error, "errno=%d\n", errno);
        exit(-1);
    }
}


//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
void I_GetSfx(sfxinfo_t * sfx)
{
    byte *dssfx;
    int size, i;
    unsigned char *paddedsfx;
    int paddedsize;

    S_GetSfxLump( sfx );
    dssfx = (byte *) sfx->data;
    if( ! dssfx )  return;
    size = sfx->length;

    paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;
    paddedsfx = (unsigned char *) Z_Malloc(paddedsize + 8, PU_STATIC, 0);
    memcpy(paddedsfx, dssfx, size);
    for (i = size; i < paddedsize + 8; i++)
        paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free(dssfx);
    *((int *) paddedsfx) = paddedsize;
    // convert raw data and header from Doom sfx to a SAMPLE for Allegro
    // write data to llsndserv 19990201 by Kin
#ifdef SNDSERV
    if (sndserver)
    {
        fputc('v', sndserver);
        fputc((char) hw_sndvolume, sndserver);
        fputc('l', sndserver);
        fwrite(paddedsfx, sizeof(int), 1, sndserver);
        fwrite(paddedsfx + 8, 1, paddedsize, sndserver);
        fflush(sndserver);
    }
    Z_Free(paddedsfx);
    paddedsfx = (unsigned char *) Z_Malloc(sizeof(int), PU_STATIC, 0);
    *((int *) paddedsfx) = sndcnt;
    sndcnt++;
#endif
    sfx->data = (void *) paddedsfx;
    sfx->length = size;
}

void I_FreeSfx(sfxinfo_t * sfx)
{
    // normal free
}


//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
int addsfx(int sfxid, int volume, int step, int seperation)
{
    int i;
    int slot;

    int rightvol;
    int leftvol;

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (S_sfx[sfxid].flags & SFX_single)
    {
        // Loop all channels, check.
        for (i = 0; i < cv_numChannels.value; i++)
        {
            // Active, and using the same SFX?
            if ((channels[i]) && (channelids[i] == sfxid))
            {
                if( S_sfx[sfxid].flags & SFX_id_fin )
                    return channelhandles[i];  // already have one
                // Kill, Reset.
                channels[i] = 0;
                break;
            }
        }
    }

    // Loop all channels to find oldest SFX.
    slot = 0;  // default
    int oldest = MAXINT;
    for (i = 0; (i < cv_numChannels.value); i++)
    {
        if (channels[i] == 0)  // unused
        {
            slot = i;
            break;
        }
        if (channelstart[i] < oldest)
        {
            slot = i;
            oldest = channelstart[i];
        }
    }

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char *) S_sfx[sfxid].data + 8;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + *((int *) S_sfx[sfxid].data);

    // Set stepping
    // Kinda getting the impression this is never used.
    channelstep[slot] = step;
    channelstepremainder[slot] = 0;
    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    leftvol = volume - ((volume * seperation * seperation) >> 16);      ///(256*256);
    seperation = seperation - 257;
    rightvol = volume - ((volume * seperation * seperation) >> 16);

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

    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[(leftvol * hw_sndvolume / 31) * 256];
    channelrightvol_lookup[slot] = &vol_lookup[(rightvol * hw_sndvolume / 31) * 256];

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = sfxid;

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    channelhandles[slot] = slot | ((channelhandles[slot] + NUM_CHANNELS) & ~CHANNEL_NUM_MASK);
    return channelhandles[slot];
}

//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux
// version.
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

    // Okay, reset internal mixing channels to zero.
    /*
       for (i=0; i<cv_numChannels.value; i++)
       {
       channels[i] = 0;
       }
     */

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
                vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
            else
                vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127 * 4;
        }
    }
}

void I_SetSfxVolume(int volume)
{
    // Identical to DOS.
    // Basically, this should propagate
    //  the menu/config file setting
    //  to the state variable used in
    //  the mixing.
    // dummy for Linux 19990117 by Kin
//  snd_SfxVolume = volume;
    hw_sndvolume = volume;

#ifdef SNDSERV
    if (sndserver)
    {
        fputc('v', sndserver);
        fputc((char) hw_sndvolume, sndserver);
        fflush(sndserver);
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
        unsigned char scmd[4];
        scmd[0] = (unsigned char) *((int *) S_sfx[sfxid].data);
        scmd[1] = (unsigned char) pitch;
        scmd[2] = (unsigned char) vol;
        scmd[3] = (unsigned char) sep;
        fputc('p', sndserver);
        fwrite(scmd, 1, 4, sndserver);
        fflush(sndserver);
    }
    // warning: control reaches end of non-void function.
    return sfxid;
#else
    // Debug.
    //GenPrintf(EMSG_debug, "starting sound %d", id );

    // Returns a handle.
    int handle = addsfx(id, vol, steptable[pitch], sep);

    //GenPrintf(EMSG_debug, "/handle is %d\n", id );

    return handle;
#endif
}

// You need the handle returned by StartSound.
void I_StopSound(int handle)
{
#ifndef SNDSERV
    int slot = handle & CHANNEL_NUM_MASK;
    if (channelhandles[slot] == handle)
    {
            channels[i] = 0;
    }
#endif
}

int I_SoundIsPlaying(int handle)
{
#ifndef SNDSERV
    int slot = handle & CHANNEL_NUM_MASK;
    if (channelhandles[slot] == handle)
    {
#if 1
        return ( channels[chan] != NULL );

#else
        // old code
            return 1;
#endif
    }
#endif
    return 0;
}

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
    int updated = 0;

    // Mix current sound data.
    // Data, from raw sound, for right and left.
    register unsigned int sample;
    register int dl;
    register int dr;
    uint16_t  sdl;
    uint16_t  sdr;

    // Pointers in global mixbuffer, left, right, end.
    int16_t * leftout;
    int16_t * rightout;
    int16_t * leftend;
    unsigned char *bothout;

    // Step in mixbuffer, left and right, thus two.
    int step;

    // Mixing channel index.
    int chan;

    if (dedicated)
        return;

    // Left and right channel
    //  are in global mixbuffer, alternating.
    leftout = mixbuffer;
    rightout = mixbuffer + 1;
    bothout = (unsigned char *) mixbuffer;
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
        for (chan = 0; chan < cv_numChannels.value; chan++)
        {
            // Check channel, if active.
            if (channels[chan])
            {
                // we are updating the mixer buffer, set flag
                updated++;
                // Get the raw data from the channel. 
                sample = *channels[chan];
                // Add left and right part
                //  for this channel (sound)
                //  to the current data.
                // Adjust volume accordingly.
                dl += channelleftvol_lookup[chan][sample];
                dr += channelrightvol_lookup[chan][sample];
                // Increment index ???
                channelstepremainder[chan] += channelstep[chan];
                // MSB is next sample???
                channels[chan] += channelstepremainder[chan] >> 16;
                // Limit to LSB???
                channelstepremainder[chan] &= 65536 - 1;

                // Check whether we are done.
                if (channels[chan] >= channelsend[chan])
                    channels[chan] = 0;
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
        if (flag)
        {
            misses += flag;
            flag = 0;
        }

        if (misses > 10)
        {
            GenPrintf(EMSG_warn, "I_SoundUpdate: missed 10 buffer writes\n");
            misses = 0;
        }

        // Increment flag for update.
        flag++;
    }
}

// 
// This is used to write out the mixbuffer
//  during each game loop update.
//
void I_SubmitSound(void)
{
    if (dedicated)
        return;

    // Write it to DSP device.
    if (flag)
    {
        if (!audio_8bit_flag)
            write(audio_fd, mixbuffer, SAMPLECOUNT * BUFMUL);
        else
            write(audio_fd, mixbuffer, SAMPLECOUNT);
        flag = 0;
    }
}

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
        for (i = 0; i < cv_numChannels.value && !channels[i]; i++)
            ;
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
    }
    else
        GenPrintf(EMSG_error, "Could not start sound server [%s]\n", fn_snd);
#else

    int i, j;

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

#ifndef OLD_SOUND_DRIVER
    myioctl(audio_fd, SNDCTL_DSP_RESET, 0);
#endif

    if (getenv("DOOM_SOUND_SAMPLEBITS") == NULL)
    {
        myioctl(audio_fd, SNDCTL_DSP_GETFMTS, &i);
        if (i &= AFMT_S16_LE)
        {
            j = 11 | (2 << 16);
            myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &j);
            myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
            i = 1;
            myioctl(audio_fd, SNDCTL_DSP_STEREO, &i);
        }
        else
        {
            i = 10 | (2 << 16);
            myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
            i = AFMT_U8;
            myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
            audio_8bit_flag++;
        }
    }
    else
    {
        i = 10 | (2 << 16);
        myioctl(audio_fd, SNDCTL_DSP_SETFRAGMENT, &i);
        i = AFMT_U8;
        myioctl(audio_fd, SNDCTL_DSP_SETFMT, &i);
        audio_8bit_flag++;
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
  "-dM", // Midi
  "-dT", // TiMidity
  "-dL", // FluidSynth
  "-dE", // Ext Midi
  "-dS", // Synth
  "-dF", // FM Synth
  "-dA"  // Awe32 Synth
};

void I_SetMusicOption(void)
{
    byte bi = cv_musserver_opt.value;

    if( msg_id < 0 )  return;
    if( bi > 6 )  return;

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
int I_PlayServerSong( char * name, int lumpnum, byte looping )
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
            int genmidi_lumpnum = W_GetNumForName( "GENMIDI" );
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
int I_RegisterSong( void* data, int len )
{
    if (nomusic)
    {
        return 1;
    }

    data = NULL;
    return 1;
}

void I_PlaySong(int handle, int looping)
{
    music_dies = gametic + (TICRATE * 30);

    if (nomusic)
        return;

    music_looping = looping;
    handle = 0;
}
#endif


// Is the song playing?
int I_QrySongPlaying(int handle)
{
    handle = 0;  // UNUSED
    return music_looping || (music_dies > gametic);
}


void I_StartupSound()
{
   if( dedicated )
       return;

   if(! nosoundfx)
       LX_InitSound();
   if(! nomusic )
       LX_InitMusic();
}

void I_ShutdownSound(void)
{
   LX_ShutdownSound();
   LX_ShutdownMusic();
}


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
void I_HandleSoundTimer(int ignore)
{
    // Debug.
    //GenPrintf(EMSG_debug, "%c", '+' ); fflush( stderr );

    // Feed sound device if necesary.
    if (flag)
    {
        // See I_SubmitSound().
        // Write it to DSP device.
        if (!audio_8bit_flag)
            write(audio_fd, mixbuffer, SAMPLECOUNT * BUFMUL);
        else
            write(audio_fd, mixbuffer, SAMPLECOUNT);

        // Reset flag counter.
        flag = 0;
    }
    else
        return;

    // UNUSED, but required.
    ignore = 0;
    return;
}

// Get the interrupt. Set duration in millisecs.
int I_SoundSetTimer(int duration_of_tick)
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
void I_SoundDelTimer()
{
    // Debug.
    if (I_SoundSetTimer(0) == -1)
        GenPrintf(EMSG_debug, "I_SoundDelTimer: failed to remove interrupt. Doh!\n");
}

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
