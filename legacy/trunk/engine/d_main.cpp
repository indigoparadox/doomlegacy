// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2007 by DooM Legacy Team.
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
#include "p_setup.h"

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


void GenerateTables();
void SV_Init();
void CL_Init();

#ifndef SVN_REV
#define SVN_REV "none"
#endif

// Version number: major.minor.revision
const int  LEGACY_VERSION = 199;  // major*100 + minor
const int  LEGACY_REVISION = 0; // for bugfix releases, should not affect compatibility
const char LEGACY_VERSIONSTRING[] = "alpha5 (rev " SVN_REV ")";
char LEGACY_VERSION_BANNER[80];


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



// "Mission packs". Used only during startup.
enum gamemission_t
{
  gmi_shareware, // DOOM 1 shareware (E1M9)
  gmi_doom1,     // registered (E3M27)
  gmi_ultimate,  // retail (Ultimate DOOM) (E4M36)
  gmi_doom2,   // DOOM 2, default
  gmi_tnt,     // TNT Evilution mission pack
  gmi_plut,    // Plutonia Experiment pack
  gmi_heretic,
  gmi_hexen
};

static gamemission_t mission = gmi_doom2;


// Helper function: start a new game
void BeginGame(int episode, int skill, bool public_server)
{
  if (public_server)
    COM.AppendText(va("newgame %s server %d %d\n", game.mapinfo_lump.c_str(), episode, skill));
  else
    COM.AppendText(va("newgame %s local %d %d\n", game.mapinfo_lump.c_str(), episode, skill));
}


//======================================================================
// EVENT HANDLING
//
// Events are asynchronous inputs generally generated by the game user.
// Events can be discarded if no responder claims them
// referenced from i_system.c for I_GetKey()
//======================================================================

event_t  events[MAXEVENTS];
int      eventhead = 0;
int      eventtail = 0;

bool shiftdown = false, altdown = false;

//
// Called by the I/O functions when input is detected
//
void D_PostEvent(const event_t* ev)
{
  events[eventhead] = *ev;
  eventhead = (eventhead+1)&(MAXEVENTS-1);
}


//
// Send all the events of the given timestamp down the responder chain
//
void D_ProcessEvents()
{
  for ( ; eventtail != eventhead ; eventtail = (eventtail+1)&(MAXEVENTS-1) )
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

// Decide between Doom or Ultimate Doom, use size to detect which one
static gamemission_t D_GetDoomType(const char *wadname)
{
  struct stat sbuf;
  // Fab: and if I patch my main wad and the size gets
  // bigger ? uh?
  // BP: main wad MUST not be patched !
  stat(wadname, &sbuf);
  if (sbuf.st_size<12408292)
    return gmi_doom1;
  else
    return gmi_ultimate;      // Ultimate
}


// identifies the iwad used
static void D_IdentifyVersion()
{
  // Specify the name of an IWAD file to use.
  // Internally the game makes no difference between IWADs and PWADs.
  // Non-free files are just not offered for upload in network games.
  // The -iwad parameter just means that we MUST have this wad file
  // in order to continue. It is also loaded right after legacy.wad.

#define NUM_IWADS 9
  struct {
    const char    *wadname;
    gamemode_t     mode;
    gamemission_t  mission;
  } iwads[NUM_IWADS] = { // order of preference
    {"doom2.wad",    gm_doom2, gmi_doom2},
    {"doomu.wad",    gm_doom1, gmi_ultimate},
    {"doom.wad",     gm_doom1, gmi_doom1},
    {"heretic.wad",  gm_heretic, gmi_heretic},
    {"hexen.wad",    gm_hexen, gmi_hexen},
    {"tnt.wad",      gm_doom2, gmi_tnt},
    {"plutonia.wad", gm_doom2, gmi_plut},
    {"doom1.wad",    gm_doom1, gmi_shareware},
    {"heretic1.wad", gm_heretic, gmi_heretic},
  };

  // default
  game.mode = gm_doom2;
  mission   = gmi_doom2;

  if (M_CheckParm("-iwad"))
    {
      const char *s = M_GetNextParm();

      if (!fc.Access(s))
	I_Error("IWAD %s not found!\n", s);

      D_AddFile(s);

      // point to start of filename only
      s = FIL_StripPath(s);

      // try to find implied gamemode
      for (int i=0; i<NUM_IWADS; i++)
	if (!strcasecmp(s, iwads[i].wadname))
	  {
	    game.mode = iwads[i].mode;
	    mission   = iwads[i].mission;
	    break;
	  }

      // Ultimate doom or not?
      if (mission == gmi_doom1)
	mission = D_GetDoomType(s);
    }
  // TODO perhaps we should not try to find a wadfile here, rather
  // start the game without any preloaded wadfiles other than legacy.wad
  else // finally we'll try to find a wad file by ourselves
    {
      int i;
      // try to find implied gamemode
      for (i=0; i<NUM_IWADS; i++)
	if (fc.Access(iwads[i].wadname))
	  {
	    D_AddFile(iwads[i].wadname);
	    game.mode = iwads[i].mode;
	    mission   = iwads[i].mission;
	    break;
	  }

      if (i == NUM_IWADS)
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


static void DoomPatchEngine()
{
  cv_jumpspeed.Set("6.0");
  cv_fallingdamage.Set("0");

  // hacks: teleport fog, blood, gibs
  mobjinfo[MT_TFOG].spawnstate = &states[S_TFOG];
  mobjinfo[MT_IFOG].spawnstate = &states[S_IFOG];
  sprnames[SPR_BLUD] = "BLUD";
  states[S_GIBS].sprite = SPR_POL5;

  // linedef conversions
  int lump = fc.GetNumForName("XDOOM");
  linedef_xtable = (xtable_t *)fc.CacheLumpNum(lump, PU_STATIC);
  linedef_xtable_size = fc.LumpLength(lump) / sizeof(xtable_t);
}


static void HereticPatchEngine()
{
  cv_jumpspeed.Set("6.0");
  cv_fallingdamage.Set("0");

  // hacks
  mobjinfo[MT_TFOG].spawnstate = &states[S_HTFOG1];
  mobjinfo[MT_IFOG].spawnstate = &states[S_HTFOG1];
  sprnames[SPR_BLUD] = "BLOD";
  states[S_GIBS].sprite = SPR_BLOD;

  int lump = fc.GetNumForName("XHERETIC");
  linedef_xtable = (xtable_t *)fc.CacheLumpNum(lump, PU_STATIC);
  linedef_xtable_size = fc.LumpLength(lump) / sizeof(xtable_t);

  // Above, good. Below, bad.
  text[TXT_PD_REDK] = "YOU NEED A GREEN KEY TO OPEN THIS DOOR";

  text[TXT_GOTBLUECARD] = "BLUE KEY";
  text[TXT_GOTYELWCARD] = "YELLOW KEY";
  text[TXT_GOTREDCARD] = "GREEN KEY";
}


static void HexenPatchEngine()
{
  cv_jumpspeed.Set("9.0");
  cv_fallingdamage.Set("23");

  // hacks
  mobjinfo[MT_TFOG].spawnstate = &states[S_XTFOG1];
  mobjinfo[MT_IFOG].spawnstate = &states[S_XTFOG1];
  sprnames[SPR_BLUD] = "BLOD";
  states[S_GIBS].sprite = SPR_GIBS;
}



//
// D_DoomMain
//
bool D_DoomMain()
{
  sprintf(LEGACY_VERSION_BANNER, "Doom Legacy %d.%d.%d %s", LEGACY_VERSION/100, LEGACY_VERSION%100, LEGACY_REVISION, LEGACY_VERSIONSTRING);

  if (M_CheckParm("--version"))
    {
      printf("%s\n", D_MakeTitleString(LEGACY_VERSION_BANNER));
      return false;
    }

  // TODO: Better commandline help
  if (M_CheckParm("--help") || M_CheckParm("-h"))
    {
      printf("%s\n", D_MakeTitleString(LEGACY_VERSION_BANNER));
      printf("Usage: legacy [-opengl] [-iwad xxx.wad] [-file pwad.wad stuff.zip ...]\n");
      return false;
    }

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
  CONS_Printf("%s\n", D_MakeTitleString(LEGACY_VERSION_BANNER));

  // get parameters from a response file (eg: legacy @parms.txt)
  // adds parameters found within file to myargc, myargv.
  M_FindResponseFile();

  // set up correct paths to wads, configfiles and saves
  D_SetPaths();

  // external Legacy data file (load it before iwad!)
  D_AddFile("legacy.wad");

  // identify the main IWAD file to use (if any),
  // set game.mode, mission accordingly
  D_IdentifyVersion();

  // game title
  const char *Titles[] =
  {
    //"No game mode chosen.",
    "DOOM Shareware Startup",
    "DOOM Registered Startup",
    "The Ultimate DOOM Startup",
    "DOOM 2: Hell on Earth",
    "DOOM 2: TNT - Evilution",
    "DOOM 2: Plutonia Experiment",
    "Heretic: Shadow of the Serpent Riders",
    "Hexen: Beyond Heretic"
  };

  const char *title = Titles[mission];

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

  // memory management
  Z_Init(); 

  // file cache
  if (!fc.InitMultipleFiles(startupwadfiles))
    I_Error("A WAD file was not found\n");

  // see that legacy.wad version matches program version
  if (!M_CheckParm("-noversioncheck"))
    D_CheckWadVersion();

  // command buffer
  COM.Init();

  // system-specific stuff
  I_SysInit();

  // generate a couple of lookup tables
  GenerateTables();

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

  // Convert mobjinfo table to DECORATE class dictionary.
  // NOTE: After this mobjinfo is not used at all! Hence DEHACKED patches etc. must be applied before this.
  ConvertMobjInfo();

  // all consvars are now registered
  //------------------------------------- CONFIG.CFG
  // loads and executes config file
  M_FirstLoadConfig(); // WARNING : this does a "COM_BufExecute()"

  if (!game.dedicated)
    {
      // set user default mode or mode set at cmdline
      vid.CheckDefaultMode();
    }

  // ------------- starting the game ----------------

  char *m;

  if (fc.FindNumForName("MAPINFO") >= 0)
    m = "MAPINFO";
  else switch (game.mode)
    {
    case gm_hexen:
      I_Error("No MAPINFO lump found!\n");
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

  game.mapinfo_lump = m;

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
    COM.AppendText(va("load %d\n", atoi(myargv[p+1])));
  else if (autostart)
    BeginGame(episode, sk, public_server);
  else
    game.StartIntro(); // start up intro loop

  // push all "+" parameter into the command buffer (they are not yet executed!)
  M_PushSpecialParameters();

  // user settings
  COM.AppendText("exec autoexec.cfg\n");

  // execute all the waiting commands in the buffer
  COM.BufExecute();

  // end of loading screen: CONS_Printf() will no more call FinishUpdate()
  con.refresh = false;
  vid.SetMode(); // change video mode if needed, recalculate...
  return true;
}
