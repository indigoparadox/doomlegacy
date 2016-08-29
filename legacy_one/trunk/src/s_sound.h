// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
// $Log: s_sound.h,v $
// Revision 1.9  2002/09/12 20:10:51  hurdler
// Added some cvars
//
// Revision 1.8  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.7  2001/02/24 13:35:21  bpereira
//
// Revision 1.6  2000/11/02 17:50:10  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.5  2000/05/13 19:52:10  metzgermeister
// cd vol jiggle
//
// Revision 1.4  2000/04/21 08:23:47  emanne
// To have SDL working.
// qmus2mid.h: force include of qmus2mid_sdl.h when needed.
// s_sound.h: with it.
//
// Revision 1.3  2000/03/12 23:21:10  linuxcub
// Added consvars which hold the filenames and arguments which will be used
// when running the soundserver and musicserver (under Linux). I hope I
// didn't break anything ... Erling Jacobsen, linuxcub@email.dk
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      The not so system specific sound interface.
//
//-----------------------------------------------------------------------------


#ifndef S_SOUND_H
#define S_SOUND_H

#include "doomdef.h"
  // SNDSERV, MUSSERV, CDMUS, MACOS_DI
#include "sounds.h"
  // sfxinfo_t
#include "command.h"
  // consvar_t

// killough 4/25/98: mask used to indicate sound origin is player item pickup
#define PICKUP_SOUND (0x8000)

extern consvar_t stereoreverse;

extern consvar_t cv_soundvolume;
extern consvar_t cv_musicvolume;
extern consvar_t cv_numChannels;
extern consvar_t cv_rndsoundpitch;

#ifdef SNDSERV
extern consvar_t cv_sndserver_cmd;
extern consvar_t cv_sndserver_arg;
#endif
#ifdef MUSSERV
extern consvar_t cv_musserver_cmd;
extern consvar_t cv_musserver_arg;
extern consvar_t cv_musserver_opt;
#endif

extern CV_PossibleValue_t soundvolume_cons_t[];
#ifdef CDMUS
//part of i_cdmus.c
extern consvar_t cd_volume;
extern consvar_t cdUpdate;
#endif

#ifdef MACOS_DI        //mp3 playlist stuff
// specific to macos directory
typedef enum
{
    music_normal,
    playlist_random,
    playlist_normal
} playmode_t;

extern consvar_t  play_mode;
#endif

// Register sound vars and commands at game startup.
void S_Register_SoundStuff (void);


//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//
void S_Init (int sfxVolume, int musicVolume);


//
// Per level startup code.
// Kills playing sounds at start of level,
//  determines music if any, changes music.
//
void S_StopSounds(void);
void S_StartSounds(void);

// [WDJ] Common routine for handling sfx names
void S_GetSfxLump( sfxinfo_t * sfx );
// [WDJ] Common routine to Free data for a sfx
void S_FreeSfx( sfxinfo_t * sfx );

// Used by hardware sound.
// Easier to pass by default instead of making it conditional.
typedef enum 
{
    CT_NORMAL = 0,
    CT_ATTACK,
    CT_SCREAM,
    CT_AMBIENT
} channel_type_t;

//
// Start sound for thing at <origin>
//  using <sound_id> from sounds.h
//
void S_StartSound(const void* origin, int sound_id );

// Special cases of 3D sources
void S_StartAmbientSound(sfxid_t sfx_id, int volume);
void S_StartAttackSound(const void *origin, sfxid_t sfx_id);
void S_StartScreamSound(const void *origin, sfxid_t sfx_id);

// Will start a sound at a given volume.
void S_StartSoundAtVolume(const void *origin_p, sfxid_t sfx_id, int volume,
			  channel_type_t ct_type );

// Stop sound for thing at <origin>
void S_StopSound(void* origin);


// Start music using <music_id> from sounds.h
void S_StartMusic(int music_id);

// Start music using <music_id> from sounds.h.
//   looping : non-zero if continuous looping of music
void S_ChangeMusic (int music_num, byte looping);
void S_ChangeMusicName(char *name, byte looping);

// Stops the music fer sure.
void S_StopMusic(void);

// Stop and resume music, during game PAUSE.
void S_PauseSound(void);
void S_ResumeSound(void);


//
// Updates music & sounds
//
void S_UpdateSounds(void);

//  volume : volume control,  0..31
void S_SetMusicVolume(int volume);
//  volume : volume control,  0..31
void S_SetSfxVolume(int volume);

//   origin : the object to check,  if NULL do not check it
//   sfxid : the sfx to check,  if sfx_None do not check it
// returns true if either is found.
boolean  S_SoundPlaying(void *origin, sfxid_t sfxid);

void S_StartSoundName(void *mo, char *soundname);

#endif
