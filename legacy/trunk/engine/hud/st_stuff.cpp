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
// $Log$
// Revision 1.2  2002/12/03 10:20:08  smite-meister
// HUD rationalized
//
// Revision 1.18  2002/09/25 15:17:39  vberghol
// Intermission fixed?
//
// Revision 1.15  2002/09/05 14:12:16  vberghol
// network code partly bypassed
//
// Revision 1.14  2002/08/24 11:57:27  vberghol
// d_main.cpp is better
//
// Revision 1.13  2002/08/23 18:05:38  vberghol
// idiotic segfaults fixed
//
// Revision 1.11  2002/08/17 16:02:04  vberghol
// final compile for engine!
//
// Revision 1.10  2002/08/13 19:47:44  vberghol
// p_inter.cpp done
//
// Revision 1.9  2002/08/08 12:01:30  vberghol
// pian engine on valmis!
//
// Revision 1.8  2002/08/06 13:14:26  vberghol
// ...
//
// Revision 1.7  2002/07/15 20:52:40  vberghol
// w_wad.cpp (FileCache class) finally fixed
//
// Revision 1.6  2002/07/12 19:21:39  vberghol
// hop
//
// Revision 1.5  2002/07/10 19:57:02  vberghol
// g_pawn.cpp tehty
//
// Revision 1.4  2002/07/01 21:00:38  jpakkane
// Fixed cr+lf to UNIX form.
//
// Revision 1.3  2002/07/01 15:01:55  vberghol
// HUD alkaa olla kunnossa
//
// Revision 1.21  2001/08/20 21:37:35  hurdler
// fix palette in splitscreen + hardware mode
//
// Revision 1.20  2001/08/20 20:40:39  metzgermeister
// *** empty log message ***
//
// Revision 1.19  2001/08/08 20:34:43  hurdler
// Big TANDL update
//
// Revision 1.18  2001/05/16 21:21:14  bpereira
// no message
//
// Revision 1.17  2001/04/01 17:35:07  bpereira
// no message
//
// Revision 1.16  2001/03/03 06:17:34  bpereira
// no message
//
// Revision 1.15  2001/02/24 13:35:21  bpereira
// no message
//
// Revision 1.14  2001/02/10 13:05:45  hurdler
// no message
//
// Revision 1.13  2001/01/31 17:14:07  hurdler
// Add cv_scalestatusbar in hardware mode
//
// Revision 1.12  2001/01/25 22:15:44  bpereira
// added heretic support
//
// Revision 1.11  2000/11/02 19:49:37  bpereira
// no message
//
// Revision 1.10  2000/10/04 16:34:51  hurdler
// Change a little the presentation of monsters/secrets numbers
//
// Revision 1.9  2000/10/02 18:25:45  bpereira
// no message
//
// Revision 1.8  2000/10/01 10:18:19  bpereira
// no message
//
// Revision 1.7  2000/10/01 01:12:00  hurdler
// Add number of monsters and secrets in overlay
//
// Revision 1.6  2000/09/28 20:57:18  bpereira
// no message
//
// Revision 1.5  2000/09/25 19:28:15  hurdler
// Enable Direct3D support as OpenGL
//
// Revision 1.4  2000/09/21 16:45:09  bpereira
// no message
//
// Revision 1.3  2000/08/31 14:30:56  bpereira
// no message
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      Status bar code.
//      Does the face/direction indicator animatin.
//      Does palette indicators as well (red pain/berserk, bright pickup)
//
//-----------------------------------------------------------------------------

#include "doomdef.h"

#include "am_map.h"

#include "d_event.h"
#include "g_game.h"
#include "g_player.h"
#include "g_pawn.h"

#include "d_netcmd.h"

#include "screen.h"
#include "r_state.h"
#include "r_main.h"

#include "m_random.h"

#include "st_lib.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"

#include "w_wad.h"
#include "z_zone.h"

#ifdef HWRENDER
# include "hardware/hw_drv.h"
# include "hardware/hw_main.h"
#endif

// buffer for drawing status bar
int fgbuffer = FG;

// Palette indices.
// For damage/bonus red-/gold-shifts
#define STARTREDPALS            1
#define STARTBONUSPALS          9
#define NUMREDPALS              8
#define NUMBONUSPALS            4
// Radiation suit, green shift.
#define RADIATIONPAL            13

// N/256*100% probability
//  that the normal face state will change
#define ST_FACEPROBABILITY              96

// For Responder
#define ST_TOGGLECHAT           KEY_ENTER

// Location of status bar
  //added:08-01-98:status bar position changes according to resolution.
#define ST_FX                     143

// Number of status faces.
#define ST_NUMPAINFACES         5
#define ST_NUMSTRAIGHTFACES     3
#define ST_NUMTURNFACES         2
#define ST_NUMSPECIALFACES      3

#define ST_FACESTRIDE \
          (ST_NUMSTRAIGHTFACES+ST_NUMTURNFACES+ST_NUMSPECIALFACES)

#define ST_NUMEXTRAFACES        2

#define ST_NUMFACES \
          (ST_FACESTRIDE*ST_NUMPAINFACES+ST_NUMEXTRAFACES)

#define ST_TURNOFFSET           (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET           (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET       (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET        (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE              (ST_NUMPAINFACES*ST_FACESTRIDE)
#define ST_DEADFACE             (ST_GODFACE+1)

#define ST_EVILGRINCOUNT        (2*TICRATE)
#define ST_STRAIGHTFACECOUNT    (TICRATE/2)
#define ST_TURNCOUNT            (1*TICRATE)
#define ST_OUCHCOUNT            (1*TICRATE)
#define ST_RAMPAGEDELAY         (2*TICRATE)

#define ST_MUCHPAIN             20



//faB: unused stuff from the Doom alpha version ?
// pistol
//#define ST_WEAPON0X           110
//#define ST_WEAPON0Y           (st_y+4)
// shotgun
//#define ST_WEAPON1X           122
//#define ST_WEAPON1Y           (st_y+4)
// chain gun
//#define ST_WEAPON2X           134
//#define ST_WEAPON2Y           (st_y+4)
// missile launcher
//#define ST_WEAPON3X           110
//#define ST_WEAPON3Y           (st_y+13)
// plasma gun
//#define ST_WEAPON4X           122
//#define ST_WEAPON4Y           (st_y+13)
// bfg
//#define ST_WEAPON5X           134
//#define ST_WEAPON5Y           (st_y+13)

// WPNS title
//#define ST_WPNSX              109
//#define ST_WPNSY              (st_y+23)

 // DETH title
//#define ST_DETHX              109
//#define ST_DETHY              (st_y+23)

//Incoming messages window location
// #define ST_MSGTEXTX     (viewwindowx)
// #define ST_MSGTEXTY     (viewwindowy+viewheight-18)
//#define ST_MSGTEXTX             0
//#define ST_MSGTEXTY             0     //added:08-01-98:unused
// Dimensions given in characters.
//#define ST_MSGWIDTH             52
// Or shall I say, in lines?
//#define ST_MSGHEIGHT            1

//#define ST_OUTTEXTX             0
//#define ST_OUTTEXTY             6

// Width, in characters again.
//#define ST_OUTWIDTH             52
// Height, in lines.
//#define ST_OUTHEIGHT            1

//#define ST_MAPWIDTH (strlen(mapnames[(gameepisode-1)*9+(gamemap-1)]))

//added:24-01-98:unused ?
//#define ST_MAPTITLEX  (vid.width - ST_MAPWIDTH * ST_CHATFONTWIDTH)

//#define ST_MAPTITLEY            0
//#define ST_MAPHEIGHT            1



//==================================
//  Legacy status bar overlay
//

static patch_t *sbohealth;
static patch_t *sbofrags;
static patch_t *sboarmor;
static patch_t *sboammo[NUMWEAPONS];

//==================================
// Doom status bar graphics
//

// doom.wad number sets:
// "AMMNUM0", small, thin, gray, no minus
// "WINUM0", large, minus is base-2, '%' is base-1

// "STTNUM0", large, red, minus is base-1, '%' is base+10
static patch_t *tallnum[11]; // 0-9, tall numbers, STTMINUS
static patch_t *tallpercent; // tall % sign

// "STYSNUM0", small, yellow, no minus
static patch_t *shortnum[11];

// "STGNUM0", small, dark gray, no minus
static patch_t *PatchArms[6][2]; // weapon ownership patches

static patch_t *PatchArmsBack; // arms background
static patch_t *PatchFaces[ST_NUMFACES]; // marine face patches
static patch_t *PatchFaceBack; // face background

static patch_t *PatchKeys[NUMCARDS]; // 3 key-cards, 3 skulls
static patch_t *PatchSTATBAR;

//==================================
// Heretic status bar graphics
//

static patch_t *PatchGod[2];
static patch_t *PatchBARBACK;
static patch_t *PatchLTFCTOP;
static patch_t *PatchRTFCTOP;
static patch_t *PatchARMCLEAR;
//static patch_t *PatchBLACKSQ;

static patch_t *Patch_InvBar[13];
static patch_t *PatchARTI[11];
static patch_t *PatchAmmoPic[7];
static patch_t *Patch_ChainSlider[5];

// numbers
patch_t *PatchSmNum[11];
patch_t *PatchINum[11];
patch_t *PatchBNum[11];

int playpalette;
int spinbooklump; // frames 0-15
int spinflylump;

patch_t *PatchFlight[16];
patch_t *PatchBook[16];

static void ST_LoadHereticData()
{
  const char patcharti[11][10] =
  {
    {"ARTIBOX"},    // none
    {"ARTIINVU"},   // invulnerability
    {"ARTIINVS"},   // invisibility
    {"ARTIPTN2"},   // health
    {"ARTISPHL"},   // superhealth
    {"ARTIPWBK"},   // tomeofpower
    {"ARTITRCH"},   // torch
    {"ARTIFBMB"},   // firebomb
    {"ARTIEGGC"},   // egg
    {"ARTISOAR"},   // fly
    {"ARTIATLP"}    // teleport
  };

  // small ammopics
  const char ammopic[7][10] =
  {
    {"BLACKSQ"}, // no ammopic for fists
    {"INAMGLD"},
    {"INAMBOW"},
    {"INAMBST"},
    {"INAMRAM"},
    {"INAMPNX"},
    {"INAMLOB"}
  };

  int i;
  int startLump;

  // gargoyle eyes
  PatchGod[0] = fc.CachePatchName("GOD1", PU_STATIC);
  PatchGod[1] = fc.CachePatchName("GOD2", PU_STATIC);

  PatchBARBACK = fc.CachePatchName("BARBACK", PU_STATIC);

  if (cv_deathmatch.value)
    PatchSTATBAR = fc.CachePatchName("STATBAR", PU_STATIC);
  else
    PatchSTATBAR = fc.CachePatchName("LIFEBAR", PU_STATIC);

  PatchLTFCTOP = fc.CachePatchName("LTFCTOP", PU_STATIC);
  PatchRTFCTOP = fc.CachePatchName("RTFCTOP", PU_STATIC);
  PatchARMCLEAR  = fc.CachePatchName("ARMCLEAR", PU_STATIC);

  // inventory bar pics
  Patch_InvBar[0] = fc.CachePatchName("INVBAR", PU_STATIC); 
  Patch_InvBar[1] = fc.CachePatchName("ARTIBOX", PU_STATIC);
  Patch_InvBar[2] = fc.CachePatchName("SELECTBOX", PU_STATIC);
  Patch_InvBar[3] = fc.CachePatchName("INVGEML1", PU_STATIC);
  Patch_InvBar[4] = fc.CachePatchName("INVGEML2", PU_STATIC);
  Patch_InvBar[5] = fc.CachePatchName("INVGEMR1", PU_STATIC);
  Patch_InvBar[6] = fc.CachePatchName("INVGEMR2", PU_STATIC);
  Patch_InvBar[7] = fc.CachePatchName("BLACKSQ", PU_STATIC); // useful? 

  // artifact use flash
  startLump = fc.GetNumForName("USEARTIA");
  for (i=0; i<5; i++) Patch_InvBar[i+8] = fc.CachePatchNum(startLump + i, PU_STATIC);

  // artifact inventory pics
  for (i=0; i < 11; i++) PatchARTI[i] = fc.CachePatchName(patcharti[i], PU_STATIC);

  // ammo pics
  for (i=0; i < 7; i++) PatchAmmoPic[i] = fc.CachePatchName(ammopic[i], PU_STATIC);

  // keys
  PatchKeys[0] = PatchKeys[3] = fc.CachePatchName("BKEYICON", PU_STATIC);
  PatchKeys[1] = PatchKeys[4] = fc.CachePatchName("YKEYICON", PU_STATIC);
  PatchKeys[2] = PatchKeys[5] = fc.CachePatchName("GKEYICON", PU_STATIC);

  // health chain slider
  Patch_ChainSlider[0] = fc.CachePatchName("CHAINBACK", PU_STATIC);
  Patch_ChainSlider[1] = fc.CachePatchName("CHAIN", PU_STATIC);
  if (!game.multiplayer)
    // single player game uses red life gem
    Patch_ChainSlider[2] = fc.CachePatchName("LIFEGEM2", PU_STATIC);
  else
    Patch_ChainSlider[2] = fc.CachePatchNum(fc.GetNumForName("LIFEGEM0") + consoleplayer->number%4, PU_STATIC);  
  Patch_ChainSlider[3] = fc.CachePatchName("LTFACE", PU_STATIC);
  Patch_ChainSlider[4] = fc.CachePatchName("RTFACE", PU_STATIC);

  // INum
  startLump = fc.GetNumForName("IN0");
  for (i = 0; i < 10; i++)
    PatchINum[i] = fc.CachePatchNum(startLump+i, PU_STATIC);
  PatchINum[10] = fc.CachePatchName("NEGNUM", PU_STATIC);
  // and "LAME"...

  // BNum
  startLump = fc.GetNumForName("FONTB16");
  for (i = 0; i < 10; i++)
    PatchBNum[i] = fc.CachePatchNum(startLump+i, PU_STATIC);
  PatchBNum[10] = fc.CachePatchNum(startLump-3, PU_STATIC); //("FONTB13")

  //SmNum
  startLump = fc.GetNumForName("SMALLIN0");
  for (i = 0; i < 10; i++)
    PatchSmNum[i] = fc.CachePatchNum(startLump+i, PU_STATIC);
  // no minus

  playpalette = fc.GetNumForName("PLAYPAL");
  spinbooklump = fc.GetNumForName("SPINBK0");
  spinflylump = fc.GetNumForName("SPFLY0");

  for (i=0; i<16; i++)
    {
      PatchFlight[i] = fc.CachePatchNum(spinflylump + i, PU_STATIC);
      PatchBook[i] = fc.CachePatchNum(spinbooklump + i, PU_STATIC);
    }
}


static void ST_LoadDoomData()
{
  int  i;
  char namebuf[9];

  // Load the numbers, tall and short
  for (i=0; i<10; i++)
    {
      sprintf(namebuf, "STTNUM%d", i);
      tallnum[i] = (patch_t *) fc.CachePatchName(namebuf, PU_STATIC);

      sprintf(namebuf, "STYSNUM%d", i);
      shortnum[i] = (patch_t *) fc.CachePatchName(namebuf, PU_STATIC);
    }

  tallnum[10] = (patch_t *) fc.CachePatchName("STTMINUS", PU_STATIC);
  shortnum[10] = shortnum[0]; // no minus available 

  // percent signs.
  tallpercent = (patch_t *) fc.CachePatchName("STTPRCNT", PU_STATIC);

  // key cards
  for (i=0;i<NUMCARDS;i++)
    {
      sprintf(namebuf, "STKEYS%d", i);
      PatchKeys[i] = (patch_t *)fc.CachePatchName(namebuf, PU_STATIC);
    }

  // arms background box
  PatchArmsBack = (patch_t *)fc.CachePatchName("STARMS", PU_STATIC);

  // arms ownership widgets
  for (i=0;i<6;i++)
    {
      sprintf(namebuf, "STGNUM%d", i+2);

      // gray #
      PatchArms[i][0] = (patch_t *)fc.CachePatchName(namebuf, PU_STATIC);

      // yellow #
      PatchArms[i][1] = shortnum[i+2];
    }

  // status bar background bits
  PatchSTATBAR = (patch_t *)fc.CachePatchName("STBAR", PU_STATIC);

  // the original Doom uses 'STF' as base name for all face graphics
  ST_loadFaceGraphics("STF");
}


// made separate so that skins code can reload custom face graphics
void ST_loadFaceGraphics (char *facestr)
{
  int   i,j;
  int   facenum;
  char  namelump[9];
  char* namebuf;

  //hack: make sure base face name is no more than 3 chars
  if (strlen(facestr)>3)
    facestr[3]='\0';
  strcpy (namelump, facestr);  // copy base name
  namebuf = namelump;
  while (*namebuf>' ') namebuf++;

  // face states
  facenum = 0;
  for (i=0;i<ST_NUMPAINFACES;i++)
    {
      for (j=0;j<ST_NUMSTRAIGHTFACES;j++)
        {
	  sprintf(namebuf, "ST%d%d", i, j);
	  PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
        }
      sprintf(namebuf, "TR%d0", i);        // turn right
      PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
      sprintf(namebuf, "TL%d0", i);        // turn left
      PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
      sprintf(namebuf, "OUCH%d", i);       // ouch!
      PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
      sprintf(namebuf, "EVL%d", i);        // evil grin ;)
      PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
      sprintf(namebuf, "KILL%d", i);       // pissed off
      PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
    }
  strcpy (namebuf, "GOD0");
  PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);
  strcpy (namebuf, "DEAD0");
  PatchFaces[facenum++] = fc.CachePatchName(namelump, PU_STATIC);

  // face backgrounds for different player colors
  //added:08-02-98: uses only STFB0, which is remapped to the right
  //                colors using the player translation tables, so if
  //                you add new player colors, it is automatically
  //                used for the statusbar.
  strcpy (namebuf, "B0");
  i = fc.FindNumForName(namelump);
  if (i != -1)
    PatchFaceBack = (patch_t *) fc.CachePatchNum(i, PU_STATIC);
  else
    PatchFaceBack = (patch_t *) fc.CachePatchName("STFB0", PU_STATIC);

}

static void ST_unloadData()
{
  int i;

  //faB: GlidePatch_t are always purgeable
  if (rendermode==render_soft)
    {
      // unload the numbers, tall and short
      for (i=0;i<10;i++)
        {
	  Z_ChangeTag(tallnum[i], PU_CACHE);
	  Z_ChangeTag(shortnum[i], PU_CACHE);
        }
      // unload tall percent
      Z_ChangeTag(tallpercent, PU_CACHE);
        
      // unload arms background
      Z_ChangeTag(PatchArmsBack, PU_CACHE);
        
      // unload gray #'s
      for (i=0;i<6;i++)
	Z_ChangeTag(PatchArms[i][0], PU_CACHE);
        
      // unload the key cards
      for (i=0;i<NUMCARDS;i++)
	Z_ChangeTag(PatchKeys[i], PU_CACHE);
        
      Z_ChangeTag(PatchSTATBAR, PU_CACHE);
    }

  ST_unloadFaceGraphics ();
}


// made separate so that skins code can reload custom face graphics
void ST_unloadFaceGraphics()
{
  int    i;

  //faB: GlidePatch_t are always purgeable
  if (rendermode == render_soft)
    {
      for (i=0;i<ST_NUMFACES;i++)
	Z_ChangeTag(PatchFaces[i], PU_CACHE);
        
      // face background
      Z_ChangeTag(PatchFaceBack, PU_CACHE);
    }
}



// refresh the status bar background
void HUD::ST_RefreshBackground()
{
  extern byte *translationtables;
  byte*       colormap;
  int flags = (fgbuffer & 0xffff0000) | BG;

  if (game.mode == heretic)
    {
      V_DrawScaledPatch(st_x, st_y, flags, PatchBARBACK);
      V_DrawScaledPatch(st_x+34, st_y+2, flags, PatchSTATBAR);
      //V_DrawScaledPatch(st_x+34, st_y+2, flags, PatchINVBAR);
      // background:
      V_DrawScaledPatch(st_x, st_y-10, flags, PatchLTFCTOP);
      V_DrawScaledPatch(st_x+290, st_y-10, flags, PatchRTFCTOP);
      //main
      V_DrawScaledPatch(st_x+57, st_y+13, flags, PatchARMCLEAR);
      //V_DrawScaledPatch(st_x+108, st_y+3, flags, PatchBLACKSQ);
      V_DrawScaledPatch(st_x+224, st_y+13, flags, PatchARMCLEAR);
    }
  else
    {
      // software mode copies patch to BG buffer,
      // hardware modes directly draw the statusbar to the screen
      V_DrawScaledPatch(st_x, st_y, flags, PatchSTATBAR);

      // draw the faceback for the statusbarplayer
      if (sbpawn->color==0)
	colormap = colormaps;
      else
	colormap = translationtables - 256 + (sbpawn->color<<8);

      V_DrawMappedPatch(st_x+ST_FX, st_y, flags, PatchFaceBack, colormap);
    }

  // copy the statusbar buffer to the screen
  if (rendermode == render_soft)
    V_CopyRect(0, vid.height-stbarheight, BG, vid.width, stbarheight, 0, vid.height-stbarheight, FG);
}


//=========================================
// HUD widget control variables

// inside the HUD class:
// statusbaron, mainbaron, invopen

static bool st_true = true; // for 

static bool st_notdeathmatch;
static bool st_armson; // !deathmatch && st_statusbaron
static bool st_fragson;  // deathmatch && st_statusbaron

static bool st_godmode;

static int  st_health;
static int  st_armor;
static int  st_readywp;
static int  st_readywp_ammo;

static int  st_faceindex = 0; // current marine face

// holds key-type for each key box on bar
static int  st_keyboxes[6];

// number of frags so far in deathmatch
static int  st_fragscount;

// Heretic spinning icons
static int  st_flight = -1;
static int  st_book = -1;

// used for evil grin
static bool  oldweaponsowned[NUMWEAPONS];


#define BLINKTHRESHOLD  (4*32)


static int ST_calcPainOffset()
{
  int         health;
  static int  lastcalc;
  static int  oldhealth = -1;

  health = (st_health > 100) ? 100 : st_health;

  if (health != oldhealth)
    {
      lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
      oldhealth = health;
    }
  return lastcalc;
}


// This is a not-very-pretty routine which handles
//  the face states and their timing.
// the precedence of expressions is:
//  dead > evil grin > turned head > straight ahead
//
void HUD::ST_updateFaceWidget()
{
  int         i;
  angle_t     badguyangle;
  angle_t     diffang;
  static int  lastattackdown = -1;
  static int  priority = 0;
  bool     doevilgrin;

  // count until face changes
  static int  st_facecount = 0;

  if (priority < 10)
    {
      // dead
      if (!sbpawn->health)
        {
	  priority = 9;
	  st_faceindex = ST_DEADFACE;
	  st_facecount = 1;
        }
    }

  if (priority < 9)
    {
      if (bonuscount)
        {
	  // picking up bonus
	  doevilgrin = false;

	  for (i=0;i<NUMWEAPONS;i++)
            {
	      if (oldweaponsowned[i] != sbpawn->weaponowned[i])
                {
		  doevilgrin = true;
		  oldweaponsowned[i] = sbpawn->weaponowned[i];
                }
            }
	  if (doevilgrin)
            {
	      // evil grin if just picked up weapon
	      priority = 8;
	      st_facecount = ST_EVILGRINCOUNT;
	      st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;
            }
        }

    }

  if (priority < 8)
    {
      if (damagecount && sbpawn->attacker && sbpawn->attacker != sbpawn)
        {
	  // being attacked
	  priority = 7;

	  if (sbpawn->health - st_oldhealth > ST_MUCHPAIN)
            {
	      st_facecount = ST_TURNCOUNT;
	      st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
	  else
            {
	      badguyangle = R_PointToAngle2(sbpawn->x,
					    sbpawn->y,
					    sbpawn->attacker->x,
					    sbpawn->attacker->y);

	      if (badguyangle > sbpawn->angle)
                {
		  // whether right or left
		  diffang = badguyangle - sbpawn->angle;
		  i = diffang > ANG180;
                }
	      else
                {
		  // whether left or right
		  diffang = sbpawn->angle - badguyangle;
		  i = diffang <= ANG180;
                } // confusing, aint it?


	      st_facecount = ST_TURNCOUNT;
	      st_faceindex = ST_calcPainOffset();

	      if (diffang < ANG45)
                {
		  // head-on
		  st_faceindex += ST_RAMPAGEOFFSET;
                }
	      else if (i)
                {
		  // turn face right
		  st_faceindex += ST_TURNOFFSET;
                }
	      else
                {
		  // turn face left
		  st_faceindex += ST_TURNOFFSET+1;
                }
            }
        }
    }

  if (priority < 7)
    {
      // getting hurt because of your own damn stupidity
      if (damagecount)
        {
	  if (sbpawn->health - st_oldhealth > ST_MUCHPAIN)
            {
	      priority = 7;
	      st_facecount = ST_TURNCOUNT;
	      st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;
            }
	  else
            {
	      priority = 6;
	      st_facecount = ST_TURNCOUNT;
	      st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
            }

        }

    }

  if (priority < 6)
    {
      // rapid firing
      if (sbpawn->attackdown)
        {
	  if (lastattackdown==-1)
	    lastattackdown = ST_RAMPAGEDELAY;
	  else if (!--lastattackdown)
            {
	      priority = 5;
	      st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
	      st_facecount = 1;
	      lastattackdown = 1;
            }
        }
      else
	lastattackdown = -1;

    }

  if (priority < 5)
    {
      // invulnerability
      if ((sbpawn->cheats & CF_GODMODE)
	  || sbpawn->powers[pw_invulnerability])
        {
	  priority = 4;

	  st_faceindex = ST_GODFACE;
	  st_facecount = 1;

        }

    }

  // look left or look right if the facecount has timed out
  if (!st_facecount)
    {
      st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
      st_facecount = ST_STRAIGHTFACECOUNT;
      priority = 0;
    }

  st_facecount--;
}


// was ST_updateWidgets
void HUD::UpdateWidgets()
{
  // TODO: put _all_ widget source variables here, so that we
  // may lose sbpawn anytime. Update source variables here.

  const int largeammo = 1994; // means "n/a"
  int i;

  // if sbpawn == NULL, don't update. where should we set it to NULL then?
  if (sbpawn == NULL)
    return;

  st_godmode = (sbpawn->cheats & CF_GODMODE);

  if (game.mode == heretic)
    {
      // Heretic flight icon
      if (sbpawn->powers[pw_flight] > BLINKTHRESHOLD || (sbpawn->powers[pw_flight] & 16))
	{
	  st_flight = (gametic/3) & 15;
	  // TODO stop the spinning when not in air?
	  // if (sbpawn->flags2 & MF2_FLY)
	}
      else
	st_flight = -1;
    
      // Heretic book icon
      if ((sbpawn->powers[pw_weaponlevel2] > BLINKTHRESHOLD || (sbpawn->powers[pw_weaponlevel2] & 16))
	  && !sbpawn->morphTics)
	{
	  st_book = (gametic/3) & 15;
	}
      else
	st_book = -1;
    }

  st_health = sbpawn->health;
  st_armor = sbpawn->armorpoints;
  st_readywp = sbpawn->readyweapon;

  ammotype_t atype = sbpawn->weaponinfo[sbpawn->readyweapon].ammo;
  if (atype == am_noammo)
    st_readywp_ammo = largeammo;    
  else
    st_readywp_ammo = sbpawn->ammo[atype];

  // update keycard multiple widgets
  for (i=0;i<6;i++)
    st_keyboxes[i] = (sbpawn->cards & (1<<i)) ? i : -1;

  // refresh everything if this is him coming back to life
  ST_updateFaceWidget();
}


static int st_palette = 0;

// sets the new palette based upon current values of damagecount
// and bonuscount

// was ST_doPaletteStuff
void HUD::ST_PaletteFlash()
{
  int palette;
  int cnt;
  int bzc;

  cnt = damagecount;

  // not relevant in heretic
  if (sbpawn->powers[pw_strength])
    {
      // slowly fade the berzerk out
      bzc = 12 - (sbpawn->powers[pw_strength]>>6);

      if (bzc > cnt)
	cnt = bzc;
    }
  
  if (cnt)
    {
      palette = (cnt+7)>>3;

      if (palette >= NUMREDPALS)
	palette = NUMREDPALS-1;

      palette += STARTREDPALS;
    }
  else if (bonuscount)
    {
      palette = (bonuscount+7)>>3;

      if (palette >= NUMBONUSPALS)
	palette = NUMBONUSPALS-1;

      palette += STARTBONUSPALS;
    }
  else if (sbpawn->powers[pw_ironfeet] > 4*32
	   || sbpawn->powers[pw_ironfeet]&8)
    palette = RADIATIONPAL; // not relevant in heretic
  else
    palette = 0;


  //added:28-02-98:quick hack underwater palette
  /*if (sbpawn &&
    (sbpawn->z + (cv_viewheight.value<<FRACBITS) < sbpawn->waterz))
    palette = RADIATIONPAL;*/

  if (palette != st_palette)
    {
      st_palette = palette;

#ifdef HWRENDER // not win32 only 19990829 by Kin
      if ((rendermode == render_opengl) || (rendermode == render_d3d))
        
        //faB - NOW DO ONLY IN SOFTWARE MODE, LETS FIX THIS FOR GOOD OR NEVER
        //      idea : use a true color gradient from frame to frame, because we
        //             are in true color in HW3D, we can have smoother palette change
        //             than the palettes defined in the wad

        {
	  //CONS_Printf("palette: %d\n", palette);
	  switch (palette)
	    {
	    case 0x00: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0x0); break;  // pas de changement
	    case 0x01: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff373797); break; // red
	    case 0x02: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff373797); break; // red
	    case 0x03: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff3030a7); break; // red
	    case 0x04: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff2727b7); break; // red
	    case 0x05: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff2020c7); break; // red
	    case 0x06: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff1717d7); break; // red
	    case 0x07: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff1010e7); break; // red
	    case 0x08: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff0707f7); break; // red
	    case 0x09: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
	    case 0x0a: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff70a090); break; // light green
	    case 0x0b: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff67b097); break; // light green
	    case 0x0c: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff60c0a0); break; // light green
	    case 0x0d: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xff60ff60); break; // green
	    case 0x0e: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
	    case 0x0f: HWD.pfnSetSpecialState(HWD_SET_PALETTECOLOR, 0xffff6060); break; // blue
            }
        }
      else
#endif
        {
	  if (!cv_splitscreen.value || !palette)
	    vid.SetPalette(palette);
        }
    }
}


void HUD::ST_DrawWidgets(bool r, bool o)
{
  int i;

  if (sbpawn->invTics)
    invopen = true;
  else
    invopen = false;

  mainbaron = statusbaron && !invopen;

  // when pawn is detached from player, no more update
  if (sbpawn->player)
    st_fragscount = sbpawn->player->score;

  // used by w_arms[] widgets
  st_armson = statusbaron && !cv_deathmatch.value;

  // used by w_frags widget
  st_fragson = cv_deathmatch.value && statusbaron;

  // used by the w_armsbg widget
  st_notdeathmatch = !cv_deathmatch.value;

  // and draw them
  if (o)
    for (i = overlay.size()-1; i>=0; i--)
      {
	overlay[i]->Update(r);
      }
  else
    for (i = widgets.size()-1; i>=0; i--)
      {
	widgets[i]->Update(r);
      }
}



void HUD::ST_CalcPos()
{
  if (cv_scalestatusbar.value || cv_viewsize.value >= 11)
    {
      fgbuffer = FG | V_SCALESTART; // scale patch by default
      st_scalex = vid.dupx;
      st_scaley = vid.dupy;
        
#ifdef HWRENDER
      if (rendermode != render_soft)
        {
	  st_x = 0;
	  st_y = BASEVIDHEIGHT - int(stbarheight/vid.fdupy);
        }
      else
#endif
        {
	  st_x = ((vid.width-ST_WIDTH*vid.dupx)>>1)/vid.dupx;
	  st_y = (vid.height - stbarheight)/vid.dupy; 
        }
    }
  else
    {
      st_scalex = st_scaley = 1;

      fgbuffer = FG | V_NOSCALEPATCH | V_NOSCALESTART;
      st_y = vid.height - stbarheight;
      st_x = (vid.width-ST_WIDTH)>>1;
    }
}


void HUD::CreateHereticWidgets()
{
  // st_ statusbaron, mainbaron, invopen

  int i;
  HudWidget *h;

  CONS_Printf("HUD::CHW, widgets_size = %d\n", widgets.size());  

  h = new HudMultIcon(20, 17, &statusbaron, &st_flight, PatchFlight);
  widgets.push_back(h);

  h = new HudMultIcon(300, 17, &statusbaron, &st_book, PatchBook);
  widgets.push_back(h);

  // godmode indicators
  h = new HudBinIcon(st_x+16, st_y+9, &statusbaron, &st_godmode, NULL, PatchGod[0]);
  widgets.push_back(h);
  h = new HudBinIcon(st_x+287, st_y+9, &statusbaron, &st_godmode, NULL, PatchGod[1]);
  widgets.push_back(h);

  // health slider
  h = new HudSlider(st_x, st_y+32, &statusbaron, &st_health, 0, 100, Patch_ChainSlider);
  widgets.push_back(h);

  // inventory system
  h = new HudInventory(st_x+34, st_y+9-9, &statusbaron, &invopen, &invuse, false, 7,
		       PatchSmNum, PatchARTI, Patch_InvBar, sbpawn);
  widgets.push_back(h);

  // mainbar (closed inventory shown)
  // frags / health
  if (cv_deathmatch.value)
    h = new HudNumber(st_x+61+27, st_y+12, &mainbaron, 3, &st_fragscount, PatchINum);
  else
    h = new HudNumber(st_x+61+27, st_y+12, &mainbaron, 3, &st_health, PatchINum);
  widgets.push_back(h);

  // Keys
  const int ST_KEYY[3] = {22, 6, 14};
  for (i=0; i<6; i++)
    {
      h = new HudMultIcon(st_x + 153, st_y + ST_KEYY[i%3], &mainbaron, &st_keyboxes[i], PatchKeys);
      widgets.push_back(h);
    }

  // readyweapon ammo
  h = new HudNumber(st_x + 109 + 27, st_y + 4, &mainbaron, 3, &st_readywp_ammo, PatchINum);
  widgets.push_back(h);

  // ammo type icon
  h = new HudMultIcon(st_x + 111, st_y + 14, &mainbaron, &st_readywp, PatchAmmoPic);
  widgets.push_back(h);

  // armor
  h = new HudNumber(st_x+228+27, st_y+12, &mainbaron, 3, &st_armor, PatchINum);
  widgets.push_back(h);
}

/*
  DOOM widgets:
  readyweapon ammo
  frags
  ammo * 4
  maxammo * 4
  health
  armor
  armsbg
  weapons owned * 6
  keycards * 3 (6!)
  face
 */

void HUD::CreateDoomWidgets()
{
  int i;
  HudWidget *h;

  //st_  statusbaron, armson, fragson

  // ready weapon ammo
  h = new HudNumber(st_x+44, st_y+3, &statusbaron, 3, &st_readywp_ammo, tallnum);
  widgets.push_back(h);

  // the last weapon type
  //w_ready->data = sbpawn->readyweapon;

  // frags
  h = new HudNumber(st_x+138, st_y+3, &st_fragson, 2, &st_fragscount, tallnum);
  widgets.push_back(h);

  // ammo count and maxammo (all four kinds)
  const int ST_AMMOY[4] = {5, 11, 23, 17};
  for (i=0; i<4; i++)
    {
      h = new HudNumber(st_x+288, st_y + ST_AMMOY[i], &statusbaron, 3, &sbpawn->ammo[i], shortnum);
      widgets.push_back(h);

      h = new HudNumber(st_x+314, st_y + ST_AMMOY[i], &statusbaron, 3, &sbpawn->maxammo[i], shortnum);
      widgets.push_back(h);
    }

  // health percentage
  h = new HudPercent(st_x+90, st_y+3, &statusbaron, &st_health, tallnum, tallpercent);
  widgets.push_back(h);

  // armor percentage - should be colored later
  h = new HudPercent(st_x+221, st_y+3, &statusbaron, &st_armor, tallnum, tallpercent);
  widgets.push_back(h);

  // Weapons
  // arms background
  h = new HudBinIcon(st_x+104, st_y, &statusbaron, &st_notdeathmatch, NULL, PatchArmsBack);
  widgets.push_back(h);

  // regexp  \([][a-z0-9&>_-]+\)
  // weapons owned
  for (i=0; i<6; i++)
    {
      h = new HudBinIcon(st_x+111+(i%3)*12, st_y+4+(i/3)*10, &st_armson,
			 &sbpawn->weaponowned[i+1], PatchArms[i][0], PatchArms[i][1]);
      widgets.push_back(h);
    }

  // face
  h = new HudMultIcon(st_x+143, st_y, &statusbaron, &st_faceindex, PatchFaces);
  widgets.push_back(h);

  // Key icon positions.
  const int ST_KEYY[3] = {3, 13, 23};
  for (i=0; i<6; i++)
    {  
      h = new HudMultIcon(st_x+239+(i/3)*10, st_y + ST_KEYY[i%3], &statusbaron, &st_keyboxes[i], PatchKeys);
      widgets.push_back(h);
    }
}


void HUD::ST_CreateWidgets()
{
  ST_CalcPos();

  CreateOverlayWidgets();

  for (int i = widgets.size()-1; i>=0; i--)
    delete widgets[i];
  widgets.clear();

  switch (game.mode)
    {
    case heretic:
      CreateHereticWidgets();
      break;
    default:
      CreateDoomWidgets();
      break;
    }
}

void HUD::ST_Drawer(bool refresh)
{
  statusbaron = (cv_viewsize.value<11) || automap.active;

  // status bar overlay at viewsize 11
  overlayon = (cv_viewsize.value == 11);

  //added:30-01-98:force a set of the palette by doPaletteStuff()
  if (vid.recalc)
    st_palette = -1;

  // Do red-/gold-shifts from damage/items
#ifdef HWRENDER
//25/08/99: Hurdler: palette changes is done for all players,
//                   not only player1 ! That's why this part 
//                   of code is moved somewhere else.
  if (rendermode==render_soft)
#endif
    ST_PaletteFlash(); //FIXME! why not in hardware?

  if (statusbaron)
    {
      // after ST_Start(), screen refresh needed, or vid mode change
      if (refresh || st_recalc || st_firsttime)
        {
	  if (st_recalc)  //recalc widget coords after vid mode change
            {
	      ST_CreateWidgets();
	      st_recalc = false;
            }
	  st_firsttime = false;
	  // draw status bar background to off-screen buff
	  ST_RefreshBackground();
	  // and refresh all widgets
	  ST_DrawWidgets(true, false);
	}
      else
	// Otherwise, update as little as possible
	ST_DrawWidgets(false, false);
    }
  else if (overlayon)
    {
      if (!drawscore || cv_splitscreen.value)
	{
	  //sbpawn = game.players[displayplayer];
	  ST_DrawWidgets(true, true);
	}

      if (cv_splitscreen.value)
	{
	  //sbpawn = game.players[secondarydisplayplayer];
	  //ST_DrawWidgets(true, true);
	}
    }
}





void HUD::ST_Stop()
{
  if (st_stopped)
    return;

  vid.SetPalette(0);

  st_stopped = true;
}

// was ST_Start
// sets up status bar to follow pawn p
// 'link' the statusbar display to a player, which could be
// another player than consoleplayer, for example, when you
// change the view in a multiplayer demo with F12.

void HUD::ST_Start(PlayerPawn *p)
{
  int i;

  sbpawn = p;

  if (!st_stopped)
    ST_Stop();

  ST_CreateWidgets();

  CONS_Printf("HUD::ST_Start, widgets_size = %d, overlay_size = %d\n", widgets.size(), overlay.size());
  
  st_firsttime = true;

  // used for timing
  //static unsigned int st_clock = 0;
  //st_chatstate = StartChatState;

  statusbaron = true;
  //st_oldchat = st_chat = false;
  //st_cursoron = false;

  st_palette = -1;


  if (game.mode != heretic)
    {
      st_faceindex = 0;
      st_oldhealth = -1;

      for (i=0;i<NUMWEAPONS;i++)
	oldweaponsowned[i] = sbpawn->weaponowned[i];

      //for (i=0;i<6;i++) keyboxes[i] = -1;
    }
  st_stopped = false;
  st_recalc = false;  //added:02-02-98: widgets coords have been setup
  // see ST_drawer()
}

//-------------------------------------------------------------------
// was ST_Init
//  Initializes the status bar,
//  sets the defaults border patch for the window borders.
//

void HUD::Init()
{
  extern bool dedicated;

  if (dedicated)
    return;

  HU_Init(); // temp solution, later combine hud and status bar...


  // cache the status bar overlay icons  (fullscreen mode)
  // legacy.wad stuff
  // Damn! sbo* icons are in pic_t format, not patch_t!
  // drawn using V_DrawScalePic()
  // FIXME crashes here!
  /*
  sbohealth = fc.CachePatchName("SBOHEALT", PU_STATIC);
  sbofrags  = fc.CachePatchName("SBOFRAGS", PU_STATIC);
  sboarmor  = fc.CachePatchName("SBOARMOR", PU_STATIC);

  int i;
  for (i=0;i<NUMWEAPONS;i++)
    {
      if (i>0 && i!=7)
	sboammo[i] = fc.CachePatchName(va("SBOAMMO%c",'0'+i), PU_STATIC);
      else
	sboammo[i] = NULL;
      CONS_Printf("sbo: %d\n", i);
    }
  */
  //added:26-01-98:screens[4] is allocated at videomode setup, and
  //               set at V_Init(), the first time being at SCR_Recalc()

  if (game.mode == heretic)
    ST_LoadHereticData();
  else
    ST_LoadDoomData();

  st_firsttime = true;

}


// =========================================================================
//                         STATUS BAR OVERLAY
// =========================================================================

void ST_CreateOverlay()
{
  hud.CreateOverlayWidgets();
}

consvar_t cv_stbaroverlay = {"overlay", "kahmf", CV_SAVE|CV_CALL, NULL, ST_CreateOverlay};


void ST_AddCommands()
{
  CV_RegisterVar(&cv_stbaroverlay);
}

/*
static inline int SCY( int y)
{ 
    //31/10/99: fixed by Hurdler so it _works_ also in hardware mode
    // do not scale to resolution for hardware accelerated
    // because these modes always scale by default
    y = y * vid.fdupy;     // scale to resolution
    if ( cv_splitscreen.value ) {
        y >>= 1;
        if (sbpawn != game.players[statusbarplayer])
            y += vid.height / 2;
    }
    return y;
}


static inline int SCX( int x)
{
    return x * vid.fdupx;
}
*/

// recreates the overlay widget set based on the consvar

void HUD::CreateOverlayWidgets()
{
  const char *cmds = cv_stbaroverlay.str;
  char   c;
  int    i;
  patch_t **lnum; // large numbers, 0-9, num[10] is minus sign
  patch_t **snum; // small numbers

  HudWidget *h;

  CONS_Printf("HUD::ST_CO\n");

  for (i = overlay.size()-1; i>=0; i--)
    delete overlay[i];
  overlay.clear();

  if (sbpawn == NULL)
    return;

  if (game.mode == heretic)
    {
      lnum = PatchBNum;
      snum = PatchSmNum;
    }
  else
    {
      lnum = tallnum;
      snum = tallnum; //...FIXME...
    };

  for (c = *cmds++; c; c = *cmds++)
    {
      if (c >= 'A' && c <= 'Z')
	c = c + 'a' - 'A';
      switch (c)
	{
	case 'i': // inventory
	  h = new HudInventory(st_x+34, st_y+9, &overlayon, &invopen, &invuse, true, 7,
			       snum, PatchARTI, Patch_InvBar, sbpawn);
	  overlay.push_back(h);
	  break;

	case 'h': // draw health
	  //ST_drawOverlayNum(SCX(50), SCY(198)-(16*vid.dupy), tallnum,NULL);
	  //DrBNumber(CPawn->health, 5, st_y+22);
	  h = new HudNumber(50, 198-16, &overlayon, 3, &st_health, lnum);
	  overlay.push_back(h);
	  //h = new HudBinIcon(52, 198-16, &overlayon, &st_true, NULL, sbohealth);
	  //overlay.push_back(h);
	  break;

	case 'f': // draw frags
	  //ST_drawOverlayNum(SCX(300), SCY(2), st_fragscount, tallnum,NULL);
	  //DrINumber(temp, 45, st_y+27);	  
	  h = new HudNumber(300, 2, &overlayon, 3, &st_fragscount, lnum);
	  overlay.push_back(h);
	  //h = new HudBinIcon(302, 2, &overlayon, &st_true, NULL, sbofrags);
	  //overlay.push_back(h);
	  break;

	case 'a': // draw ammo
	  //ST_drawOverlayNum(SCX(234), SCY(198)-(16*vid.dupy), sbpawn->ammo[sbpawn->weaponinfo[sbpawn->readyweapon].ammo], tallnum,NULL);
	  h = new HudNumber(234, 198-16, &overlayon, 3, &st_readywp_ammo, lnum);
	  overlay.push_back(h);
	  //h = new HudMultIcon(236, 198-16, &overlayon, &st_readywp, sboammo);
	  //overlay.push_back(h);	  
	  break;

	case 'k': // draw keys
	  for (i=0; i<6; i++)
	    {
	      //V_DrawScaledPatch(SCX(318)-(c++)*(ST_KEY0WIDTH*vid.dupx), SCY(198)-((16+8)*vid.dupy), FG | V_NOSCALESTART, PatchKeys[i]);
	      h = new HudMultIcon(318-(i/3)*10, 198-24-(i%3)*10, &overlayon, &st_keyboxes[i], PatchKeys);
	      overlay.push_back(h);
	    }
	  break;

         case 'm': // draw armor
           //ST_drawOverlayNum(SCX(300), SCY(198)-(16*vid.dupy), sbpawn->armorpoints, tallnum,NULL);
	   h = new HudNumber(300, 198-16, &overlayon, 3, &st_armor, lnum);
	   overlay.push_back(h);
	   //h = new HudBinIcon(302, 198-16, &overlayon, &st_true, NULL, sboarmor);
	   //overlay.push_back(h);
           break;

	default:
	  break;
	   /*
	     //TODO
	case 'e': // number of monster killed 
	  if ((!cv_deathmatch.value) && (!cv_splitscreen.value))
	    {
	      char buf[16];
	      sprintf(buf, "%d/%d", 0, 0); // FIXME! should be sbpawn.kills, map.kills
	      V_DrawString(SCX(318-V_StringWidth(buf)), SCY(1), V_NOSCALESTART, buf);

	    }
	  break;

	case 's': // number of secrets found
	  if ((!cv_deathmatch.value) && (!cv_splitscreen.value))
	    {
	      char buf[16];
	      sprintf(buf, "%d/%d", 0, 0); // FIXME! should be sbpawn., map.secrets
	      V_DrawString(SCX(318-V_StringWidth(buf)), SCY(11), V_NOSCALESTART, buf);
	    }
	  break;
	  case 'r': // current frame rate
	  {
	  char buf[8];
	  int framerate = 35;
	  sprintf(buf, "%d FPS", framerate);
	  V_DrawString(SCX(2), SCY(4), V_NOSCALESTART, buf);
	  }
	  break;
	  */
	}
    }
}