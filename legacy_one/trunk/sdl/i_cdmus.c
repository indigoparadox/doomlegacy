// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2011 by DooM Legacy Team.
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
// $Log: i_cdmus.c,v $
// Revision 1.3  2001/05/16 22:33:35  bock
// Initial FreeBSD support.
//
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
// DESCRIPTION:
//      cd music interface
//
//-----------------------------------------------------------------------------

// Because of WINVER redefine, put before any include that might define WINVER
#include "doomincl.h"

#include "SDL.h"

#include "i_sound.h"
#include "command.h"
#include "m_argv.h"

// [WDJ] SDL_cdrom.h is not present on some MAC SDL installs.
// This detects if SDL.h includes SDL/SDL_cdrom.h
#ifdef SDL_AUDIO_TRACK

#define MAX_CD_TRACKS 256

#define CD_OK(cd) (CD_INDRIVE((cd)->status))

static SDL_CD *cdrom = NULL;  // if non-NULL, the CD-ROM system is initialized

static boolean cd_enabled = false; // do we want cd music? changed using the cd console command
static boolean playLooping = false;
static unsigned int playTrack; // track being played
static unsigned int cdRemap[MAX_CD_TRACKS];

static void cd_volume_onchange(void);

CV_PossibleValue_t cd_volume_cons_t[]={{0,"MIN"},{31,"MAX"},{0,NULL}};
consvar_t cd_volume = {"cd_volume", "31", CV_SAVE | CV_CALL, cd_volume_cons_t, cd_volume_onchange};

static void cd_volume_onchange(void)
{
  // HACK: SDL does not support setting the CD volume.
  // Use pause instead and toggle between full and no music.

  if (!cdrom)
    return;

  if (cd_volume.value > 0 && cdrom->status == CD_PAUSED)
    I_ResumeCD();
  else if (cd_volume.value == 0 && cdrom->status == CD_PLAYING)
    I_PauseCD();
}


static Uint32 lastchk = 0;

/**************************************************************************
 *
 * function: CDAudio_GetAudioDiskInfo
 *
 * description:
 * update the SDL_CD status info behind the cdrom pointer
 * returns true if there's a cd in the drive and it's ok
 *
 **************************************************************************/
static boolean CDAudio_GetAudioDiskInfo(void)
{
  if (!cdrom)
    return false;

  CDstatus cdStatus = SDL_CDStatus(cdrom);

  if (cdStatus == CD_ERROR)
  {
      CONS_Printf("CD Error: %s\n", SDL_GetError());
      return false;
  }

  if (!CD_INDRIVE(cdStatus))
  {
      CONS_Printf("No CD in drive\n");
      return false;
  }
    
  return true;
}

/**************************************************************************
 *
 * function: StopCD
 *
 * description:
 *
 *
 **************************************************************************/
void I_StopCD(void)
{
  if (!cdrom)
    return;
    
  if (SDL_CDStop(cdrom))
    {
      CONS_Printf("CD stop failed\n");
    }
}

/**************************************************************************
 *
 * function: I_EjectCD
 *
 * description:
 *
 *
 **************************************************************************/
static void I_EjectCD(void)
{
  if (!cdrom)
    return; // no cd init'd
    
  I_StopCD();
    
  if (SDL_CDEject(cdrom))
  {
      CONS_Printf("CD eject failed\n");
  }
}

/**************************************************************************
 *
 * function: Command_Cd_f
 *
 * description:
 * handles all CD commands from the console
 *
 **************************************************************************/
static void Command_Cd_f (void)
{
    char	*command;
    int		ret;
    int		n;

    if (!cdrom)
	return;

    if (COM_Argc() < 2) {
	CONS_Printf ("cd [on] [off] [remap] [reset] [open]\n"
		     "   [info] [play <track>] [resume]\n"
		     "   [stop] [pause] [loop <track>]\n");
	return;
    }

    command = COM_Argv (1);

    if (!strncmp(command, "on", 2)) {
	cd_enabled = true;
	return;
    }

    if (!strncmp(command, "off", 3)) {
	I_StopCD();
	cd_enabled = false;
	return;
    }
	
    if (!strncmp(command, "remap", 5)) {
	ret = COM_Argc() - 2;
	if (ret <= 0) {
	  // list the mapping
	    for (n = 0; n < MAX_CD_TRACKS; n++)
		if (cdRemap[n] != n)
		    CONS_Printf("  %d -> %d\n", n, cdRemap[n]);
	    return;
	}

	// set a mapping
	for (n = 0; n < ret; n++)
	    cdRemap[n] = atoi(COM_Argv(n+2));
	return;
    }
        
    if (!strncmp(command, "reset", 5)) {
	cd_enabled = true;
	I_StopCD();
            
	for (n = 0; n < MAX_CD_TRACKS; n++)
	    cdRemap[n] = n;
	CDAudio_GetAudioDiskInfo();
	return;
    }

    // from this point on, make sure the cd is ok        
    if (!CD_OK(cdrom)) {
      if (!CDAudio_GetAudioDiskInfo()) // check if situation has changed
	{
	    CONS_Printf("No CD in player.\n");
	    return;
	}
    }

    if (!strncmp(command, "open", 4)) {
	I_EjectCD();
	return;
    }

    if (!strncmp(command, "info", 4))
    {
        CONS_Printf("%d tracks\n", cdrom->numtracks);
	if (cdrom->status == CD_PLAYING)
	    CONS_Printf("Currently %s track %d\n", playLooping ? "looping" : "playing", playTrack);
	else if (cdrom->status == CD_PAUSED)
	    CONS_Printf("Paused %s track %d\n", playLooping ? "looping" : "playing", playTrack);
	else
	  CONS_Printf("Not playing\n");
	CONS_Printf("Volume is %d\n", cd_volume.value);
	return;
    }

    if (!strncmp(command, "play", 4)) {
	I_PlayCD(atoi(COM_Argv (2)), false);
	return;
    }

    if (!strncmp(command, "loop", 4)) {
	I_PlayCD(atoi(COM_Argv (2)), true);
	return;
    }

    if (!strncmp(command, "stop", 4)) {
	I_StopCD();
	return;
    }
        
    if (!strncmp(command, "pause", 5)) {
	I_PauseCD();
	return;
    }
        
    if (!strncmp(command, "resume", 6)) {
	I_ResumeCD();
	return;
    }
        
    CONS_Printf("Invalid command \"cd %s\"\n", COM_Argv (1));
}


/**************************************************************************
 *
 * function: PauseCD
 *
 * description:
 *
 *
 **************************************************************************/
void I_PauseCD (void)
{
  if (!cdrom || !cd_enabled)
    return;
    
  if (SDL_CDPause(cdrom))
  {
      CONS_Printf("CD pause failed\n");
  }
}

/**************************************************************************
 *
 * function: ResumeCD
 *
 * description:
 *
 *
 **************************************************************************/
// continue after a pause
void I_ResumeCD (void)
{
  if (!cdrom || !cd_enabled)
    return;

  if (SDL_CDResume(cdrom))
  {
      CONS_Printf("CD resume failed\n");
  }
}


/**************************************************************************
 *
 * function: ShutdownCD
 *
 * description:
 *
 *
 **************************************************************************/
void I_ShutdownCD (void)
{
  if (!cdrom)
    return;

  I_StopCD();

  SDL_CDClose(cdrom);
  cdrom = NULL;
}

/**************************************************************************
 *
 * function: InitCD
 *
 * description:
 * Initialize the first CD drive SDL detects and add console command 'cd'
 *
 **************************************************************************/
void I_InitCD (void)
{
  int i;
  cdrom = NULL;

  // Initialize SDL cdrom subsystem
  if (SDL_InitSubSystem(SDL_INIT_CDROM) < 0)
    {
      CONS_Printf(" Couldn't initialize SDL CD-ROM subsystem: %s\n", SDL_GetError());
      return;
    }

  if (SDL_CDNumDrives() < 1)
    {
      CONS_Printf(" No CD-ROM drives found.\n");
      return;
    }

  // Open a drive
  const char *cdName = SDL_CDName(0);
  cdrom = SDL_CDOpen(0);
    
  if (!cdrom)
    {
      if (!cdName)
	CONS_Printf("Couldn't open default CD-ROM drive: %s\n", SDL_GetError());
      else
	CONS_Printf("Couldn't open default CD-ROM drive %s: %s\n", cdName, SDL_GetError());
	
      return;
    }
  else
    {
      CONS_Printf("Default CD-ROM drive %s initialized.\n", cdName);
    }

  // init track mapping
  for (i = 0; i < MAX_CD_TRACKS; i++)
    cdRemap[i] = i;
    
  cd_enabled = true;

  if (CDAudio_GetAudioDiskInfo())
    CONS_Printf(" %d tracks found.\n", cdrom->numtracks);

  COM_AddCommand ("cd", Command_Cd_f);
    
  CONS_Printf("CD audio initialized.\n");
}



//
/**************************************************************************
 *
 * function: UpdateCD
 *
 * description:
 * checks the cd status and re-initiates play evey 2 seconds in case the song has elapsed and we are looping
 *
 **************************************************************************/
void I_UpdateCD (void)
{
  if (!cdrom || !cd_enabled)
    return;
    
  if (cdrom->status == CD_PLAYING && lastchk < SDL_GetTicks()) 
    {
      lastchk = SDL_GetTicks() + 2000; //two seconds between chks

      // check the status
      if (!CDAudio_GetAudioDiskInfo())
	return; // no valid cd in drive

      if (cdrom->status == CD_STOPPED && playLooping)
	I_PlayCD(playTrack, true);
    }
}



/**************************************************************************
 *
 * function: PlayCD
 *
 * description:
 * play the requested track and set the looping flag
 * pauses the CD if volume is 0
 * 
 **************************************************************************/

void I_PlayCD (unsigned int track, boolean looping)
{
  if (!cdrom || !cd_enabled)
    return;
    
  if (!CD_OK(cdrom))
    {
      if (!CDAudio_GetAudioDiskInfo()) // check if situation has changed
	{
	    CONS_Printf("No CD in drive.\n");
	    return;
	}
    }

  track = cdRemap[track];
    
  if (track >= cdrom->numtracks)
    {
      CONS_Printf("I_PlayCD: Bad track number %d.\n", track);
      return;
    }
    
  // don't try to play a non-audio track
  if (cdrom->track[track].type == SDL_DATA_TRACK)
    {
      CONS_Printf("I_PlayCD: track %d is not audio.\n", track);
      return;
    }
	
  if (cdrom->status == CD_PLAYING)
    {
      if (playTrack == track)
	return; // already playing it

      I_StopCD();
    }
    
  if (SDL_CDPlayTracks(cdrom, track, 0, 1, 0))
    {
      CONS_Printf("Error playing track %d: %s\n", track, SDL_GetError());
      return;
    }
    
  playLooping = looping;
  playTrack = track;

  if (cd_volume.value == 0)
    I_PauseCD(); // cd "volume" hack
}


#else
// [WDJ] some MAC do not have SDL_cdrom.h
// This will stop errors for now.

CV_PossibleValue_t cd_volume_cons_t[]={{0,"MIN"},{31,"MAX"},{0,NULL}};
consvar_t cd_volume = {"cd_volume", "31", CV_SAVE | CV_CALL, cd_volume_cons_t, NULL};

void I_StopCD(void)
{
}

void I_PauseCD (void)
{
}

void I_ResumeCD (void)
{
}

void I_ShutdownCD (void)
{
}

void I_InitCD (void)
{
}

void I_UpdateCD (void)
{
}

void I_PlayCD (unsigned int track, boolean looping)
{
}

#endif
