// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2005 by DooM Legacy Team.
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
//
//-----------------------------------------------------------------------------

/// \file
/// \brief Startup and initialization (D_DoomMain),
/// game loop (D_DoomLoop), event system.


#include <unistd.h>
#include <sys/stat.h>

#include "command.h"
#include "console.h"
#include "cvars.h"

#include "dstrings.h"
#include "info.h"
#include "p_heretic.h"

#include "g_game.h"
#include "d_event.h"

#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "screen.h"

#include "m_argv.h"
#include "m_menu.h"
#include "m_misc.h" // configfile

#include "sounds.h"
#include "s_sound.h"

#include "w_wad.h"
#include "z_zone.h"



void SV_Init();
void CL_Init();


// Version number: major.minor.subversion
const int  LEGACY_VERSION = 199;  // major*100 + minor
const int  LEGACY_SUBVERSION = 0; // for bugfix releases, should not affect compatibility
const char LEGACY_VERSIONSTRING[] = "alpha4";

// Name of local directory for config files and savegames
#ifdef LINUX 
# define DEFAULTDIR "/.legacy"
#elif defined(__MACOS__) || defined(__APPLE_CC__)
# define DEFAULTDIR "/Library/Application Support/DooMLegacy"
#else
# define DEFAULTDIR "/legacy"
#endif

// the file where all game vars and settings are saved
#define CONFIGFILENAME   "config.cfg"  



bool devparm    = false; // started game with -devparm
bool singletics = false; // timedemo



// "Mission packs". Used only for Evilution and Plutonia.
enum gamemission_t
{
  gmi_doom2 = 0,  // DOOM 2, default
  gmi_tnt,    // TNT Evilution mission pack
  gmi_plut    // Plutonia Experiment pack
};

int mission = gmi_doom2;


// Helper function: start a new game using the predefined MAPINFO lumps in legacy.wad...
void BeginGame(int episode, int skill, bool public_server)
{
  char *m;

  switch (game.mode)
    {
    case gm_hexen:
      m = "MAPINFO";
      break;
    case gm_heretic:
      m = "MI_HTIC";
      break;
    case gm_doom2:
      if (mission == gmi_tnt)
	m = "MI_TNT";
      else if (mission == gmi_plut)
	m = "MI_PLUT";
      else
	m = "MI_DOOM2";
      break;
    default:
      m = "MI_DOOM1";
    }

  if (public_server)
    COM_BufAddText(va("newgame %s server %d %d\n", m, episode, skill));
  else
    COM_BufAddText(va("newgame %s local %d %d\n", m, episode, skill));
}


//======================================================================
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
// referenced from i_system.c for I_GetKey()
//======================================================================

event_t  events[MAXEVENTS];
int      eventhead;
int      eventtail;

bool shiftdown = false, altdown = false;

//
// Called by the I/O functions when input is detected
//
void D_PostEvent(const event_t* ev)
{
  events[eventhead] = *ev;
  eventhead = (++eventhead)&(MAXEVENTS-1);
}


//
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents()
{
  for ( ; eventtail != eventhead ; eventtail = (++eventtail)&(MAXEVENTS-1) )
    {
      event_t *ev = &events[eventtail];

      if (game.dedicated)
	con.Responder(ev); // dedicated server only has a console interface
      else
	{
	  // Menu input
	  if (Menu::Responder(ev))
	    continue;              // menu ate the event
	  // console input
	  if (con.Responder(ev))
	    continue;              // ate the event
	  game.Responder(ev);
	}
    }
}





// =========================================================================
//   D_DoomLoop
// =========================================================================


void D_DoomLoop()
{
  // timekeeping for the game
  tic_t rendertimeout = 0; // next time the screen MUST be updated
  tic_t rendertic = 0;     // last rendered gametic
  tic_t oldtics = I_GetTics(); // current time

  // main game loop
  while (1)
    {
      // How much time has elapsed?
      tic_t now = I_GetTics();
      tic_t elapsed = now - oldtics;
      oldtics = now;
        
      // give time to the OS
      if (elapsed == 0)
	{
	  I_Sleep(1);
	  continue;
	}

#ifdef HW3SOUND
      HW3S_BeginFrameUpdate();
#endif

      // run tickers, advance game state
      game.TryRunTics(elapsed);

      if (!game.dedicated)
	{
	  if (singletics || game.tic > rendertic)
	    {
	      // render if gametics have passed since last rendering
	      rendertic = game.tic;
	      rendertimeout = now + TICRATE/17;

	      // move positional sounds, adjust volumes
	      S.UpdateSounds();
	      // Update display, next frame, with current state.
	      game.Display();
	    }
	  else if (rendertimeout < now)
	    {
	      // otherwise render if enough real time has elapsed since last rendering
	      // in case the server hang or netsplit
	      game.Display();
	    }
	}

      // check for media change, loop music..
      I_UpdateCD();

#ifdef HW3SOUND
      HW3S_EndFrameUpdate();
#endif
    }
}




//
// D_AddFile
//
static char *startupwadfiles[MAX_WADFILES];

static void D_AddFile(const char *file)
{
  static int i = 0;

  if (i >= MAX_WADFILES)
    return;

  char *newfile = (char *)malloc(strlen(file)+1);
  strcpy(newfile, file);

  startupwadfiles[i++] = newfile;
}



// ==========================================================================
// Identify the Doom version, and IWAD file to use.
// Sets 'game.mode' to determine whether registered/commmercial features are
// available (notable loading PWAD files).
// ==========================================================================

// return gamemode for Doom or Ultimate Doom, use size to detect which one
static gamemode_t D_GetDoomType(const char *wadname)
{
  struct stat sbuf;
  // Fab: and if I patch my main wad and the size gets
  // bigger ? uh?
  // BP: main wad MUST not be patched !
  stat(wadname, &sbuf);
  if (sbuf.st_size<12408292)
    return gm_doom1;
  else
    return gm_udoom;      // Ultimate
}


// identifies the iwad used
static void D_IdentifyVersion()
{
  // Specify the name of an IWAD file to use.
  // Internally the game makes no difference between IWADs and PWADs.
  // Non-free files are just not offered for upload in network games.
  // The -iwad parameter just means that we MUST have this wad file
  // in order to continue. It is also loaded right after legacy.wad.

  char *doom1wad = "doom1.wad";
  char *doomwad = "doom.wad";
  char *doomuwad = "doomu.wad";
  char *doom2wad = "doom2.wad";
  char *plutoniawad = "plutonia.wad";
  char *tntwad = "tnt.wad";
  char *hereticwad = "heretic.wad";
  char *heretic1wad = "heretic1.wad";
  char *hexenwad = "hexen.wad";

  if (M_CheckParm("-iwad"))
    {
      const char *s = M_GetNextParm();

      if (!fc.Access(s))
	I_Error("IWAD %s not found!\n", s);

      D_AddFile(s);

      // point to start of filename only
      s = FIL_StripPath(s);
      
      // try to find implied gamemode
      if (!strcasecmp(plutoniawad, s))
	{
	  game.mode = gm_doom2;
	  mission = gmi_plut;
	}
      else if (!strcasecmp(tntwad, s))
	{
	  game.mode = gm_doom2;
	  mission = gmi_tnt;
	}
      else if (!strcasecmp(hereticwad, s) || !strcasecmp(heretic1wad, s))
	game.mode = gm_heretic;
      else if (!strcasecmp(hexenwad, s))
	game.mode = gm_hexen;
      else if (!strcasecmp(doom2wad, s))
	game.mode = gm_doom2;
      else if (!strcasecmp(doomuwad, s))
	game.mode = gm_udoom;
      else if (!strcasecmp(doomwad, s))
	game.mode = D_GetDoomType(s);
      else if (!strcasecmp(doom1wad, s))
	game.mode = gm_doom1s;
      else
	game.mode = gm_doom2;
    }
  // FIXME perhaps we should not try to find a wadfile here, rather
  // start the game without any preloaded wadfiles other than legacy.wad
  else // finally we'll try to find a wad file by ourselves
    {
      if (fc.Access(doom2wad))
	{
	  game.mode = gm_doom2;
	  D_AddFile(doom2wad);
	}
      else if (fc.Access(doomuwad))
	{
	  game.mode = gm_udoom;
	  D_AddFile(doomuwad);
	}
      else if (fc.Access(doomwad))
	{
	  game.mode = D_GetDoomType(doomwad);
	  D_AddFile(doomwad);
	}
      else if (fc.Access(doom1wad))
	{
	  game.mode = gm_doom1s;
	  D_AddFile(doom1wad);
	}
      else if (fc.Access(plutoniawad))
	{
	  game.mode = gm_doom2;
	  mission = gmi_plut;
	  D_AddFile(plutoniawad);
	}
      else if (fc.Access(tntwad))
	{
	  game.mode = gm_doom2;
	  mission = gmi_tnt;
	  D_AddFile(tntwad);
	}
      else if (fc.Access(hereticwad))
	{
	  game.mode = gm_heretic;
	  D_AddFile(hereticwad);
	}
      else if (fc.Access(heretic1wad))
	{
	  game.mode = gm_heretic;
	  D_AddFile(heretic1wad);
	}
      else if (fc.Access(hexenwad))
	{
	  game.mode = gm_hexen;
	  D_AddFile(hexenwad);
	}
      else
	{
	  I_Error("Main IWAD file not found.\n"
		  "You need either doom.wad, doom1.wad, doomu.wad, doom2.wad,\n"
		  "tnt.wad, plutonia.wad, heretic.wad, heretic1.wad or hexen.wad\n"
	          "from any shareware or commercial version of Doom, Heretic or Hexen!\n");
	}
    }

  // should we enable inventory?
  game.inventory = (game.mode >= gm_heretic);
}


// ========================================================================
// Just print the nice red titlebar like the original DOOM2 for DOS.
// ========================================================================
#ifdef PC_DOS
void D_Titlebar(char *title1, char *title2)
{
    // DOOM LEGACY banner
    clrscr();
    textattr((BLUE<<4)+WHITE);
    clreol();
    cputs(title1);

    // standard doom/doom2 banner
    textattr((RED<<4)+WHITE);
    clreol();
    gotoxy((80-strlen(title2))/2,2);
    cputs(title2);
    normvideo();
    gotoxy(1,3);

}
#endif


//
//  Center the title string, then add the date and time of compilation.
//
static const char *D_MakeTitleString(const char *s)
{
  static char banner[81];
  memset(banner, ' ', sizeof(banner));

  int i;

  for (i = (80 - strlen(s)) / 2; *s; )
    banner[i++] = *s++;

  char *u = __DATE__;
  for (i = 0; i < 11; i++)
    banner[i + 1] = u[i]; 

  u = __TIME__;
  for (i = 0; i < 8; i++)
    banner[i + 71] = u[i];

  banner[80] = '\0';
  return banner;
}

static void D_CheckWadVersion()
{
  // check version of legacy.wad using version lump
  int wadversion = 0;
  int lump = fc.FindNumForName("VERSION", true);
  if (lump != -1)
    {
      char s[128];
      int  l = fc.ReadLumpHeader(lump, s, 127);
      s[l]='\0';
      if (sscanf(s, "Doom Legacy WAD v%d.%d.%*d", &l, &wadversion) == 2)
	wadversion += l*100;
    }

  if (wadversion != LEGACY_VERSION)
	  I_Error("Your legacy.wad file is version %d.%d, you need version %d.%d\n"
	    "Use the legacy.wad coming from the same zip file as this executable\n"
	    "\n"
	    "Use -noversioncheck to remove this check,\n"
	    "but this can cause Legacy to crash\n",
	    wadversion/100,wadversion%100,LEGACY_VERSION/100,LEGACY_VERSION%100);
}



// set up correct paths to wads, configfiles and saves
void D_SetPaths()
{
  char *wadpath = getenv("DOOMWADDIR");
  if (!wadpath)
    wadpath = I_GetWadPath();

  // store the wad path
  fc.SetPath(wadpath);

  char *userhome;

  if (M_CheckParm("-home") && M_IsNextParm())
    userhome = M_GetNextParm();
  else
    userhome = getenv("HOME");

#ifdef LINUX // user home directory
  if (!userhome)
    I_Error("Please set $HOME to your home directory\n");
#endif

  string legacyhome = ".";

  if (userhome)
    {
      // use user specific config files and saves
      legacyhome = string(userhome);
      legacyhome += DEFAULTDIR; // config files, saves here
      I_mkdir(legacyhome.c_str(), S_IRWXU);
    }

  // check for a custom config file
  if (M_CheckParm("-config") && M_IsNextParm())
    {
      strcpy(configfile, M_GetNextParm());
      CONS_Printf("Using config file '%s'\n", configfile);
    }
  else
    {
      // little hack to allow a different config file for opengl
      // may be a problem if opengl cannot really be started
      if (M_CheckParm("-opengl"))
	sprintf(configfile, "%s/gl"CONFIGFILENAME, legacyhome.c_str());
      else
	sprintf(configfile, "%s/"CONFIGFILENAME, legacyhome.c_str());
    }

  // savegame name templates
  sprintf(savegamename, "%s/%s", legacyhome.c_str(), "savegame_%d.sav");
  sprintf(hubsavename, "%s/%s", legacyhome.c_str(), "hubsave_%02d.sav");
}



//
// D_DoomMain
//
void D_DoomMain()
{
  extern bool nomusic, nosound;

  // we need to check for dedicated before initialization of some subsystems
  game.dedicated = M_CheckParm("-dedicated");

  // keep error messages until the final flush(stderr)
  //if (setvbuf(stderr,NULL,_IOFBF,1000)) CONS_Printf("setvbuf didnt work\n");
  if (!game.dedicated)
    {
	  //FIXME: these files should be placed in ~/Library/Logs/ as Legacy_%s.log
#ifndef __APPLE_CC__
      if (freopen("stdout.txt", "w", stdout) == NULL) CONS_Printf("freopen didnt work\n");
      if (freopen("stderr.txt", "w", stderr) == NULL) CONS_Printf("freopen didnt work\n");
#endif
    }

  setbuf(stdout, NULL);      // non-buffered output

  // start console output by the banner line
  char banner[81];
  sprintf(banner, LEGACY_VERSION_BANNER, LEGACY_VERSION/100,
	  LEGACY_VERSION%100, LEGACY_SUBVERSION, LEGACY_VERSIONSTRING);
  CONS_Printf("%s\n", D_MakeTitleString(banner));

  // get parameters from a response file (eg: legacy @parms.txt)
  // adds parameters found within file to myargc, myargv.
  M_FindResponseFile();

  // set up correct paths to wads, configfiles and saves
  D_SetPaths();

  // external Legacy data file (load it before iwad!)
  D_AddFile("legacy.wad");

  // identify the main IWAD file to use (if any),
  // set game.mode, game.mission accordingly
  D_IdentifyVersion();

  // game title
  const char *Titles[] =
  {
    "No game mode chosen.",
    "DOOM Shareware Startup",
    "DOOM Registered Startup",
    "The Ultimate DOOM Startup",
    "DOOM 2: Hell on Earth",
    "Heretic: Shadow of the Serpent Riders",
    "Hexen: Beyond Heretic"
  };

  const char *title = Titles[game.mode];

  if (mission == gmi_plut)
    title = "DOOM 2: Plutonia Experiment";
  else if (mission == gmi_tnt)
    title = "DOOM 2: TNT - Evilution";

  // print out game title
  CONS_Printf("%s\n\n", title);

  // "developement parameter"
  devparm = M_CheckParm("-devparm");
  if (devparm)
    CONS_Printf(D_DEVSTR);

  // add any files specified on the command line with -file to the wad list
  if (M_CheckParm("-file"))
    {
      // the parms after p are wadfile/lump names,
      // until end of parms or another - preceded parm
      while (M_IsNextParm())
	D_AddFile(M_GetNextParm());
    }

  //========================== start subsystem initializations ==========================

  // zone memory management
  Z_Init(); 

  // file cache
  if (!fc.InitMultipleFiles(startupwadfiles))
    I_Error("A WAD file was not found\n");

  // see that legacy.wad version matches program version
  if (!M_CheckParm("-noversioncheck"))
    D_CheckWadVersion();

  // command buffer
  COM_Init();

  // system-specific stuff
  CONS_Printf("SYS_Init: Init system-specific stuff.\n");
  I_SysInit();

  // Initialize the joystick subsystem.
  I_JoystickInit();

  // adapt tables to legacy needs
  P_PatchInfoTables();

  nosound = M_CheckParm("-nosound");
  nomusic = M_CheckParm("-nomusic");

  // Server init
  SV_Init();

  // Client init
  if (!game.dedicated) 
    CL_Init();

  switch (game.mode)
    {
    case gm_hexen:
      HexenPatchEngine();
      break;
    case gm_heretic:
      HereticPatchEngine();
      break;
    default:
      DoomPatchEngine();
    }

  // all consvars are now registered
  //------------------------------------- CONFIG.CFG
  // loads and executes config file
  M_FirstLoadConfig(); // WARNING : this does a "COM_BufExecute()"

  if (!game.dedicated)
    {
      // FIXME replace these with cv_fullscreen_onchange or something...
      I_PrepareVideoModeList(); // Regenerate Modelist according to cv_fullscreen
      // set user default mode or mode set at cmdline
      SCR_CheckDefaultMode();
    }

  // ------------- starting the game ----------------

  bool public_server = game.dedicated || M_CheckParm("-server");
  bool autostart = public_server; // server starts automatically
  int  episode = 1;
  skill_t sk = sk_medium;

  // get skill / episode

  int p = M_CheckParm("-skill");
  if (p && M_IsNextParm())
    {
      sk = skill_t(myargv[p+1][0]-'1');
      autostart = true;
    }

  p = M_CheckParm("-episode");
  if (p && M_IsNextParm())
    {
      episode = myargv[p+1][0]-'0';
      autostart = true;
    }

  p = M_CheckParm("-loadgame");
  if (p && M_IsNextParm())
    COM_BufAddText(va("load %d\n", atoi(myargv[p+1])));
  else if (autostart)
    BeginGame(episode, sk, public_server);
  else
    game.StartIntro(); // start up intro loop

  // push all "+" parameter into the command buffer (they are not yet executed!)
  M_PushSpecialParameters();

  // user settings
  COM_BufAddText("exec autoexec.cfg\n");

  // execute all the waiting commands in the buffer
  COM_BufExecute();

  // end of loading screen: CONS_Printf() will no more call FinishUpdate()
  con.refresh = false;
  vid.SetMode(); // change video mode if needed, recalculate...
}

// entry point for not C++ environments (C or Obj-C)
extern "C" {
	void D_RunDoom()
	{
		D_DoomMain ();
		D_DoomLoop ();
	}
}
 
