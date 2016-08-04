// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2000-2013 by Doom Legacy team
//
// This source is available for distribution and/or modification
// only under the terms of the DOOM Source Code License as
// published by id Software. All rights reserved.
//
// The source is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// FITNESS FOR A PARTICULAR PURPOSE. See the DOOM Source Code License
// for more details.
//
// $Log: i_sound.c,v $
// Revision 1.14  2007/01/28 14:45:01  chiphog
// This patch addresses 2 issues:
// * Separated the SDL and HAVE_MIXER assumptions.  We can now compile with
//   SDL=1 but without specifying HAVE_MIXER=1.
// * Fixed the playing of MIDI (from MUS) music under SDL.  One needs to
//   compile with SDL=1 and HAVE_MIXER=1 to get SDL music.  This involved
//   two things:
//   + Removed/replaced the conflicting functions explained here:
//     http://jonatkins.org/SDL_mixer/SDL_mixer.html#SEC5
//   + Backported some SDL mixer mystery magic from legacy-2
// This patch has been tested on linux with:
// - make LINUX=1                       # you get sound, but it ain't SDL
// - make LINUX=1 SDL=1                 # you get SDL sound but no music
// - make LINUX=1 SDL=1 HAVE_MIXER=1    # you get SDL sound and SDL music
// Compiling with HAVE_MIXER=1 but without SDL=1 will fail.
//
// Revision 1.13  2006/07/22 15:38:07  hurdler
// Quick fix for SDL_mixer compiling issue
//
// Revision 1.12  2004/04/18 12:53:42  hurdler
// fix Heretic issue with SDL and OS/2
//
// Revision 1.11  2003/07/13 13:16:15  hurdler
// Revision 1.10  2001/08/20 20:40:42  metzgermeister
//
// Revision 1.9  2001/05/16 22:33:35  bock
// Initial FreeBSD support.
//
// Revision 1.8  2001/05/14 19:02:58  metzgermeister
//   * Fixed floor not moving up with player on E3M1
//   * Fixed crash due to oversized string in screen message ... bad bug!
//   * Corrected some typos
//   * fixed sound bug in SDL
//
// Revision 1.7  2001/04/14 14:15:14  metzgermeister
// fixed bug no sound device
//
// Revision 1.6  2001/04/09 20:21:56  metzgermeister
// dummy for I_FreeSfx
//
// Revision 1.5  2001/03/25 18:11:24  metzgermeister
//   * SDL sound bug with swapped stereo channels fixed
//   * separate hw_trick.c now for HW_correctSWTrick(.)
//
// Revision 1.4  2001/03/09 21:53:56  metzgermeister
// Revision 1.3  2000/11/02 19:49:40  bpereira
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
//
// DESCRIPTION:
//      System interface for sound.
//
//-----------------------------------------------------------------------------

#include <math.h>
#include <unistd.h>

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_mutex.h>
#include <SDL_version.h>
#if ((SDL_MAJOR_VERSION*100)+(SDL_MINOR_VERSION*10)) < 120
# include <SDL_byteorder.h>
#else
# include <SDL_endian.h>
#endif

#ifdef HAVE_MIXER
# define  USE_RWOPS
# include <SDL_mixer.h>
#endif

#include "doomincl.h"
#include "doomstat.h"

#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "m_swap.h"
#include "w_wad.h"

#include "s_sound.h"

#include "d_main.h"
#include "z_zone.h"

#include "qmus2mid.h"



#define MIDBUFFERSIZE   128*1024

#define MUSIC_FADE_TIME 400 // ms

// The number of internal mixing channels,
//  mixing buffer, and the samplerate of the raw data.

#define DOOM_SAMPLERATE 11025 // Hz, Doom sound effects

// Needed for calling the actual sound output.
#define NUM_CHANNELS  16    // max. number of simultaneous sounds
#define CHANNEL_NUM_MASK  (NUM_CHANNELS-1)
#define SAMPLERATE    22050 // Hz
#define SAMPLECOUNT   512   // requested audio buffer size (512 means about 46 ms at 11 kHz)

typedef struct {
   
  // The channel data pointers, start and end.
  byte * data_ptr;  // NULL when inactive
  byte * data_end;

  unsigned int step;  // The channel step amount...
  unsigned int step_remainder;   // ... and a 0.16 bit remainder of last step.

  // When the channel started playing, and too many sounds
  // determine which to kill by oldest and priority.
  unsigned int age_priority;

  // The sound in channel handles,
  //  determined on registration,
  //  might be used to unregister/stop/modify,
  // Lowest bits are the channel num.
  int handle;

  // SFX id of the playing sound effect.
  // Used to catch duplicates (like chainsaw).
  int sfxid;

  // Hardware left and right channel volume lookup.
  int * leftvol_lookup;
  int * rightvol_lookup;

} mix_channel_t;

static mix_channel_t  mix_channel[ NUM_CHANNELS ];  // channel


// Pitch to stepping lookup, 16.16 fixed point
static Sint32 steptable[256];

// Volume lookups.
static int vol_lookup[128 * 256];

// Buffer for MIDI
static byte *mus2mid_buffer;

// Flags for the -nosound and -nomusic options
extern boolean nosoundfx;
extern boolean nomusic;

static boolean musicStarted = false;
static boolean soundStarted = false;

static unsigned int sound_age = 1000;  // age counter


//
// SFX API
// Note: this was called by S_Init.
// However, whatever they did in the
// old DPMS based DOS version, this
// were simply dummies in the Linux version.
// See soundserver initdata().
//
// Well... To keep compatibility with legacy doom, I have to call this in
// I_InitSound since it is not called in S_Init... (emanne@absysteme.fr)

static void I_SetChannels(void)
{
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    //  the mixing process.
    int i, j;

    if (nosoundfx)
        return;

    double base_step = (double)DOOM_SAMPLERATE / (double)SAMPLERATE;

    // This table provides step widths for pitch parameters.
    for (i = 0; i < 256; i++)
      steptable[i] = (Sint32)(base_step * pow(2.0, ((i-128) / 64.0)) * 65536.0);

    // Generates volume lookup tables
    //  which also turn the u8 samples into s16 samples.
    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 256; j++)
        {
            vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
        }
    }
}

void I_SetSfxVolume(int volume)
{
    // Can use mix_sfxvolume (0..31), or set local volume vars.
    // mix_sfxvolume = volume;
}


void I_GetSfx(sfxinfo_t * sfx)
{
    S_GetSfxLump( sfx ); // lump to sfx
    // [WDJ] If save sfx->data += 8 to skip header,
    // then would need to undo it to Free the mem.  Caused Z_Free failure.
    if( sfx->length > 8 )
    {
        sfx->length -= 8;  // length of sound
    }
}

void I_FreeSfx(sfxinfo_t * sfx)
{
    // normal Z_Free in S_FreeSfx
}

#if 0
// cleanly stop a channel
static void stop_channel( mix_channel_t * chanp )
{
    chanp->data_ptr = NULL;
    // Do not release sound lump, it gets used too often,
    // and would have to check for other sound channels using it.
}
#endif


//
// Starting a sound means adding it
//  to the current list of active sounds in the internal channels.
// Pitching (that is, increased speed of playback)
//  is set, but currently not used by mixing.
//
//  vol : volume, 0..255
// Return a channel handle.
int I_StartSound(sfxid_t sfxid, int vol, int sep, int pitch, int priority)
{
    int handle;
    mix_channel_t  *  chanp;
    int i;
    int slot;

    if (nosoundfx)
        return 0;

#ifndef HAVE_MIXER
    SDL_LockAudio();
#endif

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (S_sfx[sfxid].flags & SFX_single)
    {
        // Loop all channels, check.
        for (i = 0; i < NUM_CHANNELS; i++)
        {
	    chanp = & mix_channel[i];
            // if Active, and using the same SFX
	    if ((chanp->data_ptr) && (chanp->sfxid == sfxid))
            {
	        if( S_sfx[sfxid].flags & SFX_id_fin )
		    return chanp->handle;  // already have one
                // Kill, Reset.
                chanp->data_ptr = 0;
                break;
            }
        }
    }

    // Loop all channels to find unused channel, or oldest SFX.
    slot = 0;  // default
    int oldest = -1;
    for (i = 0; (i < NUM_CHANNELS); i++)
    {
        if (! mix_channel[i].data_ptr )  // unused
        {
	    slot = i;
	    break;
	}
        // handles sound_age wrap, by considering only diff
        register unsigned int  agpr = sound_age - mix_channel[i].age_priority;
        if (agpr > oldest)   // older
        {
            slot = i;
            oldest = agpr;
        }
    }
   
    chanp = & mix_channel[slot];  // channel to use

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    chanp->sfxid = sfxid;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data, skipping header.
    chanp->data_ptr = (unsigned char *) S_sfx[sfxid].data + 8;
    // Set pointer to end of raw data.
    chanp->data_end = chanp->data_ptr + S_sfx[sfxid].length;

    // Set stepping
    chanp->step = steptable[pitch];
    // 16.16 fixed point
    chanp->step_remainder = 0;
    // balanced between age and priority
    // low priority (higher value) increases age
    chanp->age_priority = sound_age - priority;  // age at start
    sound_age += 16;  // vrs priority 0..256

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    sep += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    //    vol *= 8;

    // vol : range 0..255
    // mix_sfxvolume : range 0..31
    vol = (vol * mix_sfxvolume) >> 7;
    // Notice : sdldoom replaced all the calls to avoid this conversion

    int leftvol = vol - ((vol * sep * sep) >> 16);      // (256*256);
    sep = sep - 257;
    int rightvol = vol - ((vol * sep * sep) >> 16);

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
    //  for this volume level
    chanp->leftvol_lookup = &vol_lookup[leftvol * 256];
    chanp->rightvol_lookup = &vol_lookup[rightvol * 256];

    // Assign current handle number.
    // Preserved so sounds could be stopped.
    handle = slot | ((chanp->handle + NUM_CHANNELS) & ~CHANNEL_NUM_MASK);
    chanp->handle = handle;

#ifndef HAVE_MIXER
    SDL_UnlockAudio();
#endif

    // Returns a handle
    return handle;
}


//   handle : the handle returned by StartSound.
void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    int slot = handle & CHANNEL_NUM_MASK;
    if( mix_channel[slot].handle == handle )
    {
        mix_channel_t  *  chanp = & mix_channel[slot];  // channel to use

        // Separation, that is, orientation/stereo.
        //  range is: 1 - 256
        sep += 1;

        // Per left/right channel.
        //  x^2 seperation,
        //  adjust volume properly.
        //    vol *= 8;
        // vol : range 0..255
        // mix_sfxvolume : range 0..31
        vol = (vol * mix_sfxvolume) >> 7;

        int leftvol = vol - ((vol * sep * sep) >> 16);      // (256*256);
        sep = sep - 257;
        int rightvol = vol - ((vol * sep * sep) >> 16);

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
        //  for this volume level
        chanp->leftvol_lookup = &vol_lookup[leftvol * 256];
        chanp->rightvol_lookup = &vol_lookup[rightvol * 256];

        // Set stepping
        chanp->step = steptable[pitch];
    }
}


//   handle : the handle returned by StartSound.
void I_StopSound(int handle)
{
    int slot = handle & CHANNEL_NUM_MASK;
    if( mix_channel[slot].handle == handle )
    {
        // outside caller should lock
#ifndef HAVE_MIXER
        SDL_LockAudio();
#endif

        mix_channel[slot].data_ptr = NULL;
//        stop_channel( & mix_channel[slot] );

#ifndef HAVE_MIXER
        SDL_UnlockAudio();
#endif
    }
}

//   handle : the handle returned by StartSound.
int I_SoundIsPlaying(int handle)
{
    int slot = handle & CHANNEL_NUM_MASK;
    if( mix_channel[slot].handle == handle )
    {
        return mix_channel[slot].data_ptr != NULL;
    }
    return 0;
}

//
// Not used by SDL version
//
void I_SubmitSound(void)
{
}

//
// This function loops all active (internal) sound
//  channels, retrieves a given number of samples
//  from the raw sound data, modifies it according
//  to the current (internal) channel parameters,
//  mixes the per channel samples into the given
//  mixing buffer, and clamping it to the allowed
//  range.
//
// This function currently supports only 16bit.
//
void I_UpdateSound(void)
{
    /*
       Pour une raison que j'ignore, la version SDL n'appelle jamais
       ce truc directement. Fonction vide pour garder une compatibilité
       avec le point de vue de legacy...
     */

    // Himmel, Arsch und Zwirn
}

static void I_UpdateSound_sdl(void *unused, Uint8 *stream, int len)
{
    int chan;
    // Mix current sound data.
    // Data, from raw sound, for right and left.
    if (nosoundfx)
        return;

    // Pointers in audio stream, left, right, end.
    // Left and right channels are multiplexed in the audio stream, alternating.
    Sint16 *leftout  = (Sint16 *)stream;
    Sint16 *rightout = leftout + 1;

    // Step in stream, left and right channels, thus two.
    int step = 2;

    // first Sint16 at least partially outside the buffer
    Sint16 *buffer_end = ((Sint16 *)stream) +len/sizeof(Sint16);

    // Mix sounds into the mixing buffer.
    while (rightout < buffer_end)
    {
        // take the current audio output (incl. music) and mix (add) in our sfx
        register int dl = *leftout;
        register int dr = *rightout;

        // Love thy L2 chache - made this a loop.
        // Now more channels could be set at compile time
        //  as well. Thus loop those  channels.
	// Mixing channel index.
        register mix_channel_t * chanp = & mix_channel[ 0 ];
        for (chan = NUM_CHANNELS; chan > 0; chan--)
        {
	    register byte * chan_data_ptr = chanp->data_ptr;
            // Check channel, if active.
            if ( chan_data_ptr )
            {
	        // Get the raw data from the channel.
	        register unsigned int sample = * chan_data_ptr;
                // Add left and right part for this channel (sound)
                //  to the current data.
                // Adjust volume accordingly.
                dl += chanp->leftvol_lookup[sample];
                dr += chanp->rightvol_lookup[sample];
		// 16.16 fixed point step forward in the sound data
                chanp->step_remainder += chanp->step;
                // take full steps
                chan_data_ptr += chanp->step_remainder >> 16;
                // remainder, save for next round
                chanp->step_remainder &= 0xFFFF;

                // Check whether we are done.
                if (chan_data_ptr >= chanp->data_end)
                    chan_data_ptr = NULL;
	        
	        chanp->data_ptr = chan_data_ptr;
            }
	    chanp ++;  // next channel
        }

        // Clamp to range. Left hardware channel.
        // Has been char instead of short.

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

        // Increment current pointers in stream
        leftout += step;
        rightout += step;
    }
}




//
// MUSIC API.
//

#ifdef HAVE_MIXER
/// the "registered" piece of music
static struct music_channel_t
{
  Mix_Music *mus;
  SDL_RWops *rwop; ///< must not be freed before music is halted
} music = { NULL, NULL };


#if ((SDL_MIXER_MAJOR_VERSION*100)+(SDL_MIXER_MINOR_VERSION*10)+SDL_MIXER_PATCHLEVEL) < 127
  // Older SDL without RWOPS
#define OLD_SDL_MIXER

#ifdef SMIF_PC_DOS
static char * midiname = "DoomMUS.mid";
#else
static char midiname[24] = "/tmp/DoomMUSXXXXXX";
#endif
FILE * midifile;

void Init_OLD_SDL_MIXER( void )
{
#ifndef SMIF_PC_DOS
    // Make temp file name
    mkstemp( midiname );
//    strcat( midiname, ".mid" );
#endif
//    fprintf( stderr, "Midiname= %s\n", midiname );
}

void Free_OLD_SDL_MIXER( void )
{
    // delete the temp file
    remove( midiname );
}


#include <errno.h>
extern int errno;

void Midifile_OLD_SDL_MIXER( byte* midibuf, unsigned long midilength )
{
    midifile = fopen( midiname, "wb" );
    if( midifile )
    {
	  fwrite( midibuf, midilength, 1, midifile );
	  fclose( midifile );
          if(verbose)
              fprintf( stderr, "Midifile written: %s size=%li\n", midiname, midilength );

          // wants file to have .MID extension, but mkstemp file cannot have extension
	  music.mus = Mix_LoadMUS( midiname );
          if( music.mus == NULL )
          {
	     I_SoftError("Music load file failed\n");
	     perror( "Mix_LoadMUS fails when not cd to doomlegacy directory" );
	  }
    }
}
#endif
#endif


void I_PlaySong(int handle, int looping)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return;

  if (music.mus)
  {
      Mix_FadeInMusic(music.mus, looping ? -1 : 1, MUSIC_FADE_TIME);
  }
#endif
}

void I_PauseSong(int handle)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return;

  Mix_PauseMusic();
#endif
}

void I_ResumeSong(int handle)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return;

  Mix_ResumeMusic();
#endif
}

void I_StopSong(int handle)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return;

  Mix_FadeOutMusic(MUSIC_FADE_TIME);
#endif
}


void I_UnRegisterSong(int handle)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return;

  if (music.mus)
  {
      Mix_FreeMusic(music.mus);
      music.mus = NULL;
      music.rwop = NULL;
  }
#endif
}


// return handle (always 0)
int I_RegisterSong(void* data, int len)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return 0;

  if (music.mus)
  {
      I_SoftError("Two registered pieces of music simultaneously!\n");
      return 0;
  }

  if (memcmp(data, MUSHEADER, 4) == 0)
  {
      unsigned long midilength;  // per qmus2mid, SDL_RWFromConstMem wants int
      // convert mus to mid in memory with a wonderful function
      // thanks to S.Bacquet for the source of qmus2mid
      int err = qmus2mid(data, mus2mid_buffer, 89, 64, 0, len, MIDBUFFERSIZE, &midilength);
      if ( err != 0 )
      {
	  CONS_Printf("Cannot convert MUS to MIDI: error %d.\n", err);
	  return 0;
      }
#ifdef OLD_SDL_MIXER
      Midifile_OLD_SDL_MIXER( mus2mid_buffer, midilength );
#else     
      music.rwop = SDL_RWFromConstMem(mus2mid_buffer, midilength);
#endif   
  }
  else
  {
      // MIDI, MP3, Ogg Vorbis, various module formats
#ifdef OLD_SDL_MIXER
      Midifile_OLD_SDL_MIXER( data, len );
#else     
      music.rwop = SDL_RWFromConstMem(data, len);
#endif   
  }

#ifdef OLD_SDL_MIXER
  // In old mixer Mix_LoadMUS_RW does not work.
#else
  // SDL_mixer automatically frees the rwop when the music is stopped.
  music.mus = Mix_LoadMUS_RW(music.rwop);
#endif   
  if (!music.mus)
  {
      CONS_Printf("Couldn't load music lump: %s\n", Mix_GetError());
      music.rwop = NULL;
  }

//  CONS_Printf("register song\n"); 	// [WDJ] debug
#endif

  return 0;
}


void I_SetMusicVolume(int volume)
{
  // volume: 0--31
#ifdef HAVE_MIXER
  if (nomusic)
    return;

  Mix_VolumeMusic((MIX_MAX_VOLUME * volume) / 32);
#endif
}




void I_StartupSound(void)
{
  static SDL_AudioSpec audspec;  // [WDJ] desc name, too many audio in this file

  if (nosoundfx)
  {
      nomusic = true;
      return;
  }

  // Configure sound device
  CONS_Printf("I_InitSound: ");

  // Open the audio device
  audspec.freq = SAMPLERATE;
  audspec.format = AUDIO_S16SYS;
  audspec.channels = 2;
  audspec.samples = SAMPLECOUNT;
  audspec.callback = I_UpdateSound_sdl;
  I_SetChannels();

#ifndef HAVE_MIXER
  // no mixer, no music
  nomusic = true;

  // Open the audio device
  if (SDL_OpenAudio(&audspec, NULL) < 0)
  {
      CONS_Printf("Couldn't open audio with desired format.\n");
      SDL_CloseAudio();
      nosoundfx = nomusic = true;
      return;
  }

  SDL_PauseAudio(0);
#else
  // use SDL_mixer for music

  // because we use SDL_mixer, audio is opened here.
  if (Mix_OpenAudio(audspec.freq, audspec.format, audspec.channels, audspec.samples) < 0)
  {
    // [WDJ] On sound cards without midi ports, opening audio will block music.
    // When midi music is played through Timidity, it will also try to use the
    // dsp port, which is already in use.  Need to use a mixer on sound
    // effect and Timidity output.  Some sound cards have two dsp ports.

        CONS_Printf("Unable to open audio: %s\n", Mix_GetError());
        nosoundfx = nomusic = true;
        return;
  }

  int number_channels;	// for QuerySpec
  if (!Mix_QuerySpec(&audspec.freq, &audspec.format, &number_channels))
  {
      CONS_Printf("Mix_QuerySpec: %s\n", Mix_GetError());
      nosoundfx = nomusic = true;
      return;
  }

  Mix_SetPostMix(audspec.callback, NULL);  // after mixing music, add sound fx
  Mix_Resume(-1); // start all sound channels (although they are not used)
#endif

  CONS_Printf("Audio device initialized: %d Hz, %d samples/slice.\n",
	      audspec.freq, audspec.samples);

#ifdef HAVE_MIXER
  if (!nomusic)
  {
      Mix_ResumeMusic();  // start music playback
      mus2mid_buffer = (byte *)Z_Malloc(MIDBUFFERSIZE, PU_STATIC, NULL);

#ifdef OLD_SDL_MIXER
  Init_OLD_SDL_MIXER();
#endif

      CONS_Printf(" Music initialized.\n");
      musicStarted = true;
  }
#endif

  // Finished initialization.
  CONS_Printf("I_InitSound: sound module ready.\n");
  soundStarted = true;

  CONS_Printf(" done.\n");
}


void I_ShutdownSound(void)
{
  if (nosoundfx || !soundStarted)
    return;

  CONS_Printf("I_ShutdownSound: ");

#ifdef HAVE_MIXER
  Mix_CloseAudio();
#else
  SDL_CloseAudio();
#endif

  CONS_Printf("shut down\n");
  soundStarted = false;

  // music
  if (musicStarted)
  {
      Z_Free(mus2mid_buffer);

#ifdef OLD_SDL_MIXER
      Free_OLD_SDL_MIXER();
#endif

      musicStarted = false;
  }
}
