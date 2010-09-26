// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 2000-2010 by Doom Legacy team
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
// go RC1
//
// Revision 1.10  2001/08/20 20:40:42  metzgermeister
// *** empty log message ***
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
// *** empty log message ***
//
// Revision 1.3  2000/11/02 19:49:40  bpereira
// no message
//
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// clean up & made it work again
//
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

#include "SDL.h"
#include "SDL_audio.h"
#include "SDL_mutex.h"
#include "SDL_byteorder.h"
#include "SDL_version.h"

#ifdef HAVE_MIXER
# define  USE_RWOPS
# include "SDL_mixer.h"
#endif

#include "z_zone.h"

#include "m_swap.h"
#include "i_system.h"
#include "i_sound.h"
#include "m_argv.h"
#include "m_misc.h"
#include "w_wad.h"

#include "doomdef.h"
#include "doomstat.h"
#include "s_sound.h"
#include "doomtype.h"

#include "d_main.h"

#include "qmus2mid.h"



#define MIDBUFFERSIZE   128*1024

#define MUSIC_FADE_TIME 400 // ms

// The number of internal mixing channels,
//  mixing buffer, and the samplerate of the raw data.

#define DOOM_SAMPLERATE 11025 // Hz, Doom sound effects

// Needed for calling the actual sound output.
#define NUM_CHANNELS  8     // max. number of simultaneous sounds
#define SAMPLERATE    22050 // Hz
#define SAMPLECOUNT   512   // requested audio buffer size (512 means about 46 ms at 11 kHz)

static int lengths[NUMSFX];     // The actual lengths of all sound effects.
static unsigned int channelstep[NUM_CHANNELS];  // The channel step amount...
static unsigned int channelstepremainder[NUM_CHANNELS]; // ... and a 0.16 bit remainder of last step.

// The channel data pointers, start and end.
static unsigned char *channels[NUM_CHANNELS];
static unsigned char *channelsend[NUM_CHANNELS];

// Time/gametic that the channel started playing,
//  used to determine oldest, which automatically
//  has lowest priority.
// In case number of active sounds exceeds
//  available channels.
static int channelstart[NUM_CHANNELS];

// The sound in channel handles,
//  determined on registration,
//  might be used to unregister/stop/modify,
//  currently unused.
static int channelhandles[NUM_CHANNELS];

// SFX id of the playing sound effect.
// Used to catch duplicates (like chainsaw).
static int channelids[NUM_CHANNELS];

// Pitch to stepping lookup, 16.16 fixed point
static Sint32 steptable[256];

// Volume lookups.
static int vol_lookup[128 * 256];

// Hardware left and right channel volume lookup.
static int *channelleftvol_lookup[NUM_CHANNELS];
static int *channelrightvol_lookup[NUM_CHANNELS];

// Buffer for MIDI
static byte *mus2mid_buffer;

// Flags for the -nosound and -nomusic options
extern boolean nosound;
extern boolean nomusic;

static boolean musicStarted = false;
static boolean soundStarted = false;

//
// This function loads the sound data from the WAD lump,
//  for single sound.
//
static void *getsfx(const char *sfxname, int *len)
{
    char name[20] = "\0\0\0\0\0\0\0\0";  // do not leave this to chance [WDJ]
    int sfxlump;

    // Get the sound data from the WAD, allocate lump
    //  in zone memory.
    if (gamemode == heretic){	// [WDJ] heretic names are different
       sprintf(name, "%s", sfxname);
    }else{
       sprintf(name, "ds%s", sfxname);
    }

    // Now, there is a severe problem with the sound handling,
    // in it is not (yet/anymore) gamemode aware. That means, sounds from
    // DOOM II will be requested even with DOOM shareware.
    // The sound list is wired into sounds.c, which sets the external variable.
    // I do not do runtime patches to that variable. Instead, we will use a
    // default sound for replacement.

    if (W_CheckNumForName(name) == -1)
    {
//	fprintf(stderr,"Sound missing: %s, Using default sound\n",name);  // [WDJ] debug
	// Heretic shareware: get many missing sound names at sound init,
	// but not after game starts.  These come from list of sounds
	// in sounds.c, but not all those are in the game.
        if (gamemode == heretic)
            sfxlump = W_GetNumForName("keyup");
        else
            sfxlump = W_GetNumForName("dspistol");
    }
    else
        sfxlump = W_GetNumForName(name);

    int size = W_LumpLength(sfxlump);
    byte *sfx = (byte *) W_CacheLumpNum(sfxlump, PU_SOUND);

    /*
      // [smite] this seems all unnecessary
    // Pads the sound effect out to the mixing buffer size.
    // The original realloc would interfere with zone memory.
    int paddedsize = ((size - 8 + (SAMPLECOUNT - 1)) / SAMPLECOUNT) * SAMPLECOUNT;

    // Allocate from zone memory.
    byte *paddedsfx = (byte *) Z_Malloc(paddedsize + 8, PU_STATIC, 0);
    // This should interfere with zone memory handling,
    //  which does not kick in in the soundserver.

    // Now copy and pad.
    memcpy(paddedsfx, sfx, size);
    int i;
    for (i = size; i < paddedsize + 8; i++)
        paddedsfx[i] = 128;

    // Remove the cached lump.
    Z_Free(sfx);

    // Preserve padded length.
    *len = paddedsize;

    // Return allocated padded data.
    return (void *) (paddedsfx + 8);
    */

    *len = size - 8; // skip header
    return sfx + 8;
}

//
// This function adds a sound to the
//  list of currently active sounds,
//  which is maintained as a given number
//  (eight, usually) of internal channels.
// Returns a handle.
//
static int addsfx(int sfxid, int volume, int step, int seperation)
{
    static unsigned short handlenums = 0;

    int i;
    int rc = -1;

    int oldest = gametic;
    int oldestnum = 0;
    int slot;

    int rightvol;
    int leftvol;

    // Chainsaw troubles.
    // Play these sound effects only one at a time.
    if (sfxid == sfx_sawup || sfxid == sfx_sawidl || sfxid == sfx_sawful || sfxid == sfx_sawhit || sfxid == sfx_stnmov || sfxid == sfx_pistol)
    {
        // Loop all channels, check.
        for (i = 0; i < NUM_CHANNELS; i++)
        {
            // Active, and using the same SFX?
            if ((channels[i]) && (channelids[i] == sfxid))
            {
                // Reset.
                channels[i] = 0;
                // We are sure that iff,
                //  there will only be one.
                break;
            }
        }
    }

    // Loop all channels to find unused channel, or oldest SFX.
    for (i = 0; (i < NUM_CHANNELS) && (channels[i]); i++)
    {
        if (channelstart[i] < oldest)
        {
            oldestnum = i;
            oldest = channelstart[i];
        }
    }

    // Tales from the cryptic.
    // If we found a channel, fine.
    // If not, we simply overwrite the first one, 0.
    // Probably only happens at startup.
    if (i == NUM_CHANNELS)
        slot = oldestnum;
    else
        slot = i;

    // Okay, in the less recent channel,
    //  we will handle the new SFX.
    // Set pointer to raw data.
    channels[slot] = (unsigned char *) S_sfx[sfxid].data;
    // Set pointer to end of raw data.
    channelsend[slot] = channels[slot] + lengths[sfxid];

    // Reset current handle number, limited to 0..100.
    if (!handlenums)
        handlenums = 100;

    // Assign current handle number.
    // Preserved so sounds could be stopped (unused).
    channelhandles[slot] = rc = handlenums++;

    // Set stepping???
    channelstep[slot] = step;
    // 16.16 fixed point
    channelstepremainder[slot] = 0;
    // Should be gametic, I presume.
    channelstart[slot] = gametic;

    // Separation, that is, orientation/stereo.
    //  range is: 1 - 256
    seperation += 1;

    // Per left/right channel.
    //  x^2 seperation,
    //  adjust volume properly.
    //    volume *= 8;

    // Volume arrives in range 0..255 and it must be in 0..cv_soundvolume...
    volume = (volume * cv_soundvolume.value) >> 7;
    // Notice : sdldoom replaced all the calls to avoid this conversion

    leftvol = volume - ((volume * seperation * seperation) >> 16);      ///(256*256);
    seperation = seperation - 257;
    rightvol = volume - ((volume * seperation * seperation) >> 16);

    // Sanity check, clamp volume.
    if (rightvol < 0 || rightvol > 127)
        I_Error("rightvol out of bounds");

    if (leftvol < 0 || leftvol > 127)
        I_Error("leftvol out of bounds");

    // Get the proper lookup table piece
    //  for this volume level???
    channelleftvol_lookup[slot] = &vol_lookup[leftvol * 256];
    channelrightvol_lookup[slot] = &vol_lookup[rightvol * 256];

    // Preserve sound SFX id,
    //  e.g. for avoiding duplicates of chainsaw.
    channelids[slot] = sfxid;

    // You tell me.
    return rc;
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
// Well... To keep compatibility with legacy doom, I have to call this in
// I_InitSound since it is not called in S_Init... (emanne@absysteme.fr)

static void I_SetChannels(void)
{
    // Init internal lookups (raw data, mixing buffer, channels).
    // This function sets up internal lookups used during
    //  the mixing process.
    int i;
    int j;

    if (nosound)
        return;

    double base_step = (double)DOOM_SAMPLERATE / (double)SAMPLERATE;

    // This table provides step widths for pitch parameters.
    for (i = 0; i < 256; i++)
      steptable[i] = (Sint32)(base_step * pow(2.0, ((i-128) / 64.0)) * 65536.0);

    // Generates volume lookup tables
    //  which also turn the u8 samples into s16 samples.
    for (i = 0; i < 128; i++)
        for (j = 0; j < 256; j++)
        {
            vol_lookup[i * 256 + j] = (i * (j - 128) * 256) / 127;
        }
}

void I_SetSfxVolume(int volume)
{
    // Identical to DOS.
    // Basically, this should propagate
    //  the menu/config file setting
    //  to the state variable used in
    //  the mixing.

    CV_SetValue(&cv_soundvolume, volume);

}

//
// Retrieve the raw data lump index
//  for a given SFX name.
//
int I_GetSfxLumpNum(sfxinfo_t * sfx)
{
    char namebuf[9];
    sprintf(namebuf, "ds%s", sfx->name);
    return W_GetNumForName(namebuf);
}

void *I_GetSfx(sfxinfo_t * sfx)
{
    int len;
    return getsfx(sfx->name, &len);
}

// FIXME: dummy for now Apr.9 2001 by Rob
void I_FreeSfx(sfxinfo_t * sfx)
{
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
int I_StartSound(int id, int vol, int sep, int pitch, int priority)
{

    // UNUSED
    priority = 0;

    if (nosound)
        return 0;

    // Returns a handle (not used).
#ifndef HAVE_MIXER
    SDL_LockAudio();
#endif
    id = addsfx(id, vol, steptable[pitch], sep);
#ifndef HAVE_MIXER
    SDL_UnlockAudio();
#endif

    return id;
}

void I_StopSound(int handle)
{
    // You need the handle returned by StartSound.
    // Would be looping all channels,
    //  tracking down the handle,
    //  an setting the channel to zero.

    handle = 0;
}

int I_SoundIsPlaying(int handle)
{
    // Ouch.
    return gametic < handle;
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
    // Mix current sound data.
    // Data, from raw sound, for right and left.
    if (nosound)
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
	int chan;
        for (chan = 0; chan < NUM_CHANNELS; chan++)
        {
            // Check channel, if active.
            if (channels[chan])
            {
	      // Get the raw data from the channel.
	      register unsigned int sample = *channels[chan];
                // Add left and right part
                //  for this channel (sound)
                //  to the current data.
                // Adjust volume accordingly.
                dl += channelleftvol_lookup[chan][sample];
                dr += channelrightvol_lookup[chan][sample];
		// 16.16 fixed point step forward in the sound data
                channelstepremainder[chan] += channelstep[chan];
                // take full steps
                channels[chan] += channelstepremainder[chan] >> 16;
                // remainder, save for next round
                channelstepremainder[chan] &= 65536 - 1;

                // Check whether we are done.
                if (channels[chan] >= channelsend[chan])
                    channels[chan] = 0;
            }
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

void I_UpdateSoundParams(int handle, int vol, int sep, int pitch)
{
    // I fail too see that this is used.
    // Would be using the handle to identify
    //  on which channel the sound might be active,
    //  and resetting the channel parameters.

    // UNUSED.
    handle = vol = sep = pitch = 0;
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

#ifdef PC_DOS
static char * midiname = "DoomMUS.mid";
#else
static char midiname[24] = "/tmp/DoomMUSXXXXXX";
#endif
FILE * midifile;

void Init_OLD_SDL_MIXER( void )
{
#ifndef PC_DOS
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

void Midifile_OLD_SDL_MIXER( byte* midibuf, unsigned long midilength )
{
    midifile = fopen( midiname, "wb" );
    if( midifile )
    {
	  fwrite( midibuf, midilength, 1, midifile );
	  fclose( midifile );
          fprintf( stderr, "Midifile written: %s size=%li\n", midiname, midilength );
	 
	  music.mus = Mix_LoadMUS( midiname );
          if( music.mus == NULL )
	     I_SoftError("Music load file failed\n");
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


int I_RegisterSong(void* data, int len)
{
#ifdef HAVE_MIXER
  if (nomusic)
    return 0;

  if (music.mus)
  {
      I_Error("Two registered pieces of music simultaneously!\n");
  }

  if (memcmp(data, MUSMAGIC, 4) == 0)
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
      music.rwop = SDL_RWFromConstMem(data, len);
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

  if (nosound)
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
      nosound = nomusic = true;
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
        nosound = nomusic = true;
        return;
    }

  int number_channels;	// for QuerySpec
  if (!Mix_QuerySpec(&audspec.freq, &audspec.format, &number_channels))
    {
      CONS_Printf("Mix_QuerySpec: %s\n", Mix_GetError());
      nosound = nomusic = true;
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


  // [smite] TODO this does not belong in the audio interface, except we need to fill the lengths array... should update S_sfx to include it
  // Initialize external data (all sounds) at start, keep static.
  CONS_Printf("Caching sound data (%d sfx)... ", NUMSFX);

    int i;
    for (i = 1; i < NUMSFX; i++)
    {
        // Alias? Example is the chaingun sound linked to pistol.
        if (S_sfx[i].name)
        {
            if (!S_sfx[i].link)
            {
                // Load data from WAD file.
                S_sfx[i].data = getsfx(S_sfx[i].name, &lengths[i]);
            }
            else
            {
                // Previously loaded already?
                S_sfx[i].data = S_sfx[i].link->data;
                lengths[i] = lengths[(S_sfx[i].link - S_sfx) / sizeof(sfxinfo_t)];
            }
        }
    }

  CONS_Printf(" done.\n");
}


void I_ShutdownSound(void)
{
  if (nosound || !soundStarted)
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
